/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file command.c
 * @brief Simple line command interpreter
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.1
 * @date 2016-11-05
 */

#include <glib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "command.h"
#include "log.h"
#include "i18n.h"

struct _Command {
    char *subcmd;
    char *argv[COMMAND_MAX_ARGS];
    char *opt_key[COMMAND_MAX_OPTS];
    char *opt_val[COMMAND_MAX_OPTS];

    CommandBind *bind;
    char *rawcmd;
};

static Command* command_new(CommandBind *bind, const char *rawcmd);
static void command_free(Command *cmd);
static SrnRet command_parse(CommandContext *ctx, Command *cmd, void *user_data);

static SrnRet get_arg(char *ptr, char **arg, char **next);
static SrnRet get_quote_arg(char *ptr, char **arg, char **next);
static SrnRet get_last_quote_arg(char *ptr, char **arg);

/**
 * @brief command_proc Check and parse command, and call the corresponding
 *      callback function
 *
 * @param ctx A CommandContext
 * @param rawcmd A string of command
 * @param user_data Custom data passing to command callback function
 *
 * @return SRN_ERR if failed, the reason is various(no such command, no enouch
 *         argument, callback function failed, etc.)
 */
SrnRet command_proc(CommandContext *ctx, const char *rawcmd, void *user_data){
    SrnRet ret;
    Command *cmd;

    if (!ctx) return SRN_ERR;

    for (int i = 0; ctx->binds[i].name != NULL; i++){
        if (g_str_has_prefix(rawcmd, ctx->binds[i].name)){
            cmd = command_new(&ctx->binds[i], rawcmd);
            ret = command_parse(ctx, cmd, user_data);
            if (ret == SRN_OK){
                // callback
                ret = cmd->bind->cb(cmd, user_data);
            } else if (ret == SRN_ERR) {
                // TODO: decorate
                ret = RET_ERR(_("Sorry, command parse failed, please report a bug to <" PACKAGE_WEBSITE "/issues> ."));
            }
            command_free(cmd);
            return ret;
        }
    }

    return RET_ERR(_("Unknown command: %s"), rawcmd);
}

/**
 * @brief command_get_subcmd Get the sub command of the command
 *
 * @param cmd
 *
 * @return NULL or sub command
 */
const char *command_get_subcmd(Command *cmd){
    g_return_val_if_fail(cmd, NULL);

    return cmd->subcmd;
}

/**
 * @brief command_get_arg Get the nth argument in the command
 *
 * @param cmd
 * @param index
 *
 * @return NULL or the nth argument, you do not need to free it
 */
const char *command_get_arg(Command *cmd, unsigned index){
    g_return_val_if_fail(cmd, NULL);

    if (index < cmd->bind->argc){
        return cmd->argv[index];
    }
    return NULL;
}

/**
 * @brief command_get_opt Whether the option is specified in the command
 *
 * @param cmd
 * @param opt_key The name of option
 * @param opt_val Can be NULL. If this option has a value, it will return via
 *          this param, you do not need to free it
 *
 * @return TRUE if this option is specified in the command,
 *         FALSE if this option is not specified in the command or there no such
 *         optional argument
 *
 * .. note::
 *
 *      Whatever the option is specified in the command, if this option
 *      have a value, it will be return to the ``opt_key``
 *
 */
bool command_get_opt(Command *cmd, const char *opt_key, const char **opt_val){
    unsigned i = 0;

    g_return_val_if_fail(cmd, FALSE);

    while (cmd->opt_key[i] != NULL){
        if (g_ascii_strcasecmp(cmd->opt_key[i], opt_key) == 0){
            if (opt_val != NULL){
                *opt_val = cmd->opt_val[i];
            }
            return TRUE;
        }
        i++;
    }

    // Option no specified in command
    i = 0;
    while (cmd->bind->opt[i].key != NULL){
        if (g_ascii_strcasecmp(cmd->bind->opt[i].key, opt_key) == 0){
            if (cmd->bind->opt[i].val == COMMAND_OPT_NO_VAL){
                // Nothing todo
            } else if (cmd->bind->opt[i].val == COMMAND_OPT_NO_DEFAULT){
                WARN_FR("No default value for opiton '%s'", opt_key);
            } else {
                if (opt_val != NULL){
                    *opt_val = cmd->bind->opt[i].val;
                }
            }
            return FALSE;
        }
        i++;
    }

    WARN_FR("No such option '%s'", opt_key);
    return FALSE;
}

static Command* command_new(CommandBind *bind, const char *rawcmd){
    Command *cmd = g_malloc0(sizeof(Command));

    cmd->bind = bind;
    cmd->rawcmd = g_strdup(rawcmd);

    g_strchomp(cmd->rawcmd); // Removes trailing whitespace

    return cmd;
}

static void command_free(Command *cmd){
    if (cmd->rawcmd)
        g_free(cmd->rawcmd);
    g_free(cmd);
}

static SrnRet get_arg(char *ptr, char **arg, char **next){
    char *arg_in;
    char *next_in;
    g_return_val_if_fail(ptr && arg && next, SRN_ERR);
    g_return_val_if_fail(*ptr != '\0', SRN_ERR);

    arg_in = strtok(ptr, " ");
    next_in = strtok(NULL, "");

    if (arg_in) {
        if (next_in){
            next_in = g_strchug(next_in);  // Remove leading whitespace of next argument
        }

        *arg = arg_in;
        *next = next_in;

        DBG_FR("arg: '%s'", *arg);
        return SRN_OK;
    }

    return SRN_ERR;
}

static SrnRet get_quote_arg(char *ptr, char **arg, char **next){
    bool quote;
    bool escape;
    char *arg_in;
    char *next_in;

    g_return_val_if_fail(ptr && arg && next, SRN_ERR);
    g_return_val_if_fail(*ptr != '\0', SRN_ERR);

    ptr = g_strchug(ptr); // Remove leading whitespace

    if (*ptr != '\'') {
        return get_arg(ptr, arg, next);
    }

    quote = TRUE;
    escape = FALSE;
    *ptr++ = '\0';
    arg_in = ptr;

    while (*ptr && quote) {
        if (!escape && *ptr == '\\'){
            escape = TRUE;
            ptr++;
        }
        else if (escape && *ptr == '\\'){
            strcpy(ptr, ptr + 1);
            escape = FALSE;
        }
        else if (escape && *ptr == '\''){
            strcpy(ptr, ptr + 1);
            escape = FALSE;
        }
        else if (!escape && *ptr == '\''){
            quote = FALSE;
            *ptr++ = '\0';
            break;
        }
        else {
            escape = FALSE;
            ptr++;
        }
    }

    if (quote){
        return RET_ERR(_("Unclosed single quotation"));
    }

    next_in = ptr;
    next_in = g_strchug(next_in);
    if (*next_in == '\0'){
        /* Reach end */
        *next = NULL;
    } else {
        *next = next_in;
    }

    *arg = arg_in;

    DBG_FR("arg: '%s'", *arg);

    return SRN_OK;
}

static SrnRet get_last_quote_arg(char *ptr, char **arg){
    g_return_val_if_fail(ptr && arg, SRN_ERR);
    g_return_val_if_fail(*ptr != '\0', SRN_ERR);

    /* Remove the leading and trailing whitespace */
    ptr = g_strchug(ptr);
    ptr = g_strchomp(ptr);

    if (*ptr == '\''){
        char *next;
        return get_quote_arg(ptr, arg, &next);
    } else {
        *arg = ptr;
    }

    DBG_FR("arg: '%s'", *arg);

    return SRN_OK;
}

static SrnRet command_parse(CommandContext *ctx, Command *cmd, void *user_data){
    int nopt = 0;
    int narg = 0;
    char *ptr, *tmp;
    SrnRet ret;
    CommandBind *bind = cmd->bind;

    /* Skip command name */
    ret = get_arg(cmd->rawcmd, &tmp, &ptr);
    g_return_val_if_fail(ret == SRN_OK, SRN_ERR);

    /* Subcommand */
    if (ptr){
        bool has_subcmd = FALSE;
        for (int i = 0; cmd->bind->subcmd[i]; i++){
            has_subcmd = TRUE;
            if (!ptr) break;
            if (!g_str_has_prefix(ptr, cmd->bind->subcmd[i])){
                continue;
            }
            /* Subcommand found */
            cmd->subcmd = ptr;
            DBG_FR("subcmd: %s", cmd->subcmd);

            get_arg(ptr, &tmp, &ptr);
        }
        if (has_subcmd && !cmd->subcmd) {
            goto unknown_subcmd;
        }
    }

    /* Get options */
    while (ptr && *ptr == COMMAND_OPT_PREFIX){
        int i;
        for (i = 0; bind->opt[i].key != NULL; i++){
            if (g_str_has_prefix(ptr, bind->opt[i].key)){
                break;
            }
        }

        if (get_arg(ptr, &cmd->opt_key[nopt], &ptr) != SRN_OK){
            ERR_FR("get option key failed");
            return SRN_ERR;
        }

        if (bind->opt[i].key == NULL){
            goto unknown_opt;
        }

        DBG_FR("Option '%s' found", cmd->opt_key[nopt]);

        /* Option has value */
        if (bind->opt[i].val != COMMAND_OPT_NO_VAL){
            if (!ptr || *ptr == '-'){
                goto missing_opt_val;
            }
            /* Get option vaule */
            if (get_quote_arg(ptr, &cmd->opt_val[nopt], &ptr) != SRN_OK){
                ERR_FR("get option val failed");
                return SRN_ERR;
            }
        } else {
            cmd->opt_val[nopt] = NULL;
        }

        if (nopt >= COMMAND_MAX_OPTS){
            goto too_many_opt;
        }

        nopt++;
    }
    cmd->opt_key[nopt] = NULL;

    /* Get arguments */
    for (int i = 0; i < bind->argc; i++){
        if (i != bind->argc - 1){
            if (get_quote_arg(ptr, &cmd->argv[narg], &ptr) != SRN_OK){
                goto missing_arg;
            }
        }  else {
            if (get_last_quote_arg(ptr, &cmd->argv[narg]) != SRN_OK){
                goto missing_arg;
            }
        }
        narg++;
        if (!ptr){
            goto missing_arg;
        }
    }

    /* Debug output */
    {
        DBG_FR("command: %s", cmd->bind->name);
        for (int i = 0; i < nopt; i++){
            DBG_FR("opt: '%s'", cmd->opt_key[i]);
            if (bind->opt[i].val != COMMAND_OPT_NO_VAL)
                DBG_FR("val: '%s'", cmd->opt_val[i]);
        }

        for (int i = 0; i < bind->argc; i++){
            DBG_FR("argv: '%s'", cmd->argv[i]);
        }
    }

    return SRN_OK;

missing_arg:
    if (cmd->bind->flag & COMMAND_FLAG_OMIT_ARG){
        return SRN_OK;
    }
    return RET_ERR(_("Missing argument, expect %d, actually %d"), cmd->bind->argc, narg);

unknown_opt:
    return RET_ERR(_("Unknown option %s"), cmd->opt_key[nopt]);

too_many_opt:
    return RET_ERR(_("Too many options, options count limit to %d"), COMMAND_MAX_OPTS);

missing_opt_val:
    return RET_ERR(_("Missing vaule for option %s"), cmd->opt_key[nopt]);

unknown_subcmd:
    return RET_ERR(_("Unknown sub command: %s"), ptr);

#if 0
too_many_arg:
    /* Currently, we regard all remaining text as the last argument, so this
     * label is never used. */
    return RET_ERR(_("Too many arguments, expect %d"), cmd->bind->argc);
#endif
}

/* inner test case */
void get_quote_arg_test() {
    char *start, *end;
    assert(get_quote_arg(strdup("'test'"), &start, &end) == SRN_OK);
    assert(strcmp(start, "test") == 0);
    assert(end == NULL);

    assert(get_quote_arg(strdup("'test''123'"), &start, &end) == SRN_OK);
    assert(strcmp(start, "test") == 0);
    assert(strcmp(end, "'123'") == 0);

    assert(get_quote_arg(strdup("test   123 4"), &start, &end) == SRN_OK);
    assert(strcmp(start, "test") == 0);
    assert(strcmp(end, "123 4") == 0);

    assert(get_quote_arg(strdup("'test"), &start, &end) == SRN_ERR);

    assert(get_quote_arg(strdup("'test\\'"), &start, &end) == SRN_ERR);

    assert(get_quote_arg(strdup("'test\\\\'"), &start, &end) == SRN_OK);
    assert(strcmp(start, "test\\") == 0);
    assert(end == NULL);
}
