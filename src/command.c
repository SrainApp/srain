/**
 * @file command.c
 * @brief Simple line command interpreter
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-11-05
 *
 * Command syntax::
 *
 *      <command> [option] [arguments...]
 *
 * A ``command`` is a string that doesn't contain whitespace. Usually it starts
 * with a slash('/'), it is not necessarhy.
 *
 * A ``option`` starts with a '-' and maybe have a value. ``option`` is optional
 * for a command, if it is specified, it should behind ``command``.
 *
 */

// #define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "command.h"
#include "srain.h"

#include "log.h"

static Command* command_new(CommandBind *bind, const char *rawcmd);
static void command_free(Command *cmd);
static int command_parse(CommandContext *ctx, Command *cmd, void *user_data);

static int get_arg(char *ptr, char **arg, char **next);
static int get_quote_arg(char *ptr, char **arg, char **next);
static int get_last_quote_arg(char *ptr, char **arg);

/**
 * @brief command_proc Check and parse command, and call the corresponding callback function
 *
 * @param ctx A CommandContext
 * @param rawcmd A string of command
 * @param user_data Custom data passing to command callback function
 *
 * @return SRN_ERR if failed, the reason is various(no such command, no enouch
 *         argument, callback function failed, etc.)
 */
int command_proc(CommandContext *ctx, const char *rawcmd, void *user_data){
    int ret;
    Command *cmd;

    if (!ctx) return -1;

    for (int i = 0; ctx->binds[i].name != NULL; i++){
        if (g_str_has_prefix(rawcmd, ctx->binds[i].name)){
            cmd = command_new(&ctx->binds[i], rawcmd);
            if (command_parse(ctx, cmd, user_data) == 0){
                // callback
                ret = cmd->bind->cb(cmd, user_data);
                if (ret < 0){
                    ctx->on_callback_fail(cmd, user_data);
                }
            } else {
                ret = -1;
            }
            command_free(cmd);
            return ret;
        }
    }

    ctx->on_unknown_cmd(rawcmd, user_data);
    return -1;
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
    if (index < cmd->bind->argc){
        return cmd->argv[index];
    }
    return NULL;
}

/**
 * @brief command_get_opt Get the optional argument in the command
 *
 * @param cmd
 * @param opt_key The name of optional argument
 * @param opt_val Can be NULL. If this argument has a value, it will return via this param,
 *        you do not need to free it
 *
 * @return 1 if this argument is specified in the command,
 *         0 if this argument is not specified in the command or there no such
 *         optional argument
 */
int command_get_opt(Command *cmd, const char *opt_key, char **opt_val){
    unsigned i = 0;

    while (cmd->opt_key[i] != NULL){
        if (strcasecmp(cmd->opt_key[i], opt_key) == 0){
            if (opt_val != NULL){
                *opt_val = cmd->opt_val[i];
            }
            return 1;
        }
        i++;
    }

    // Option no specified in command
    i = 0;
    while (cmd->bind->opt_key[i] != NULL){
        if (strcasecmp(cmd->bind->opt_key[i], opt_key) == 0){
            if (opt_val != NULL){
                *opt_val = cmd->bind->opt_default_val[i];
            }
            return 1;
        }
        i++;
    }

    ERR_FR("No such option '%s'", opt_key);
    return 0;
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

static int get_arg(char *ptr, char **arg, char **next){
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

static int get_quote_arg(char *ptr, char **arg, char **next){
    bool quote;
    bool escape;

    g_return_val_if_fail(ptr && arg && next, SRN_ERR);
    g_return_val_if_fail(*ptr != '\0', SRN_ERR);

    ptr = g_strchug(ptr); // Remove leading whitespace

    if (*ptr != '\'') {
        return get_arg(ptr, arg, next);
    }

    quote = TRUE;
    escape = FALSE;
    *ptr++ = '\0';
    *arg = ptr;

    while (*ptr && quote) {
        if (!escape && *ptr == '\\'){
            escape = TRUE;
        }
        else if (escape && *ptr == '\''){
            strcpy(ptr, ptr + 1);
        }
        else if (!escape && *ptr == '\''){
            *ptr = '\0';
            quote = FALSE;
        } else {
            escape = FALSE;
        }
        ptr++;
    }

    if (quote){
        ERR_FR("Unterminal quote");
        return SRN_ERR;
    }

    *next = ptr;
    *next = g_strchug(*next);
    if (*next == '\0'){
        /* Reach end */
        *next = NULL;
    }

    DBG_FR("arg: '%s'", *arg);

    return SRN_OK;
}

static int get_last_quote_arg(char *ptr, char **arg){
    g_return_val_if_fail(ptr && arg, SRN_ERR);
    g_return_val_if_fail(*ptr != '\0', SRN_ERR);

    /* Remove the leading and trailing whitespace */
    ptr = g_strchug(ptr);
    ptr = g_strchomp(ptr);

    if (*ptr == '\''){
        char *next;
        if (get_quote_arg(ptr, arg, &next) != SRN_OK){
            return SRN_ERR;
        }
        if (next) {
            return SRN_ERR;
        }
    }

    *arg = ptr;

    return SRN_OK;
}

static int command_parse(CommandContext *ctx, Command *cmd, void *user_data){
    int ret;
    int nopt = 0;
    char *ptr, *tmp;
    CommandBind *bind = cmd->bind;

    /* Skip command name */
    ret = get_arg(cmd->rawcmd, &tmp, &ptr);
    g_return_val_if_fail(ret == SRN_OK, SRN_ERR);

    /* Get options */
    while (ptr && *ptr == COMMAND_OPT_PREFIX){
        int i;
        for (i = 0; bind->opt_key[i] != NULL; i++){
            if (g_str_has_prefix(ptr, bind->opt_key[i])){
                break;
            }
        }

        if (get_arg(ptr, &cmd->opt_key[nopt], &ptr) != SRN_OK){
            ERR_FR("get option key failed");
            return SRN_ERR;
        }

        if (bind->opt_key[i] == NULL){
            goto unknown_opt;
        }

        DBG_FR("Option '%s' found", cmd->opt_key[nopt]);

        /* Option has value */
        if (bind->opt_default_val[i] != NULL){
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
    if (!ptr){
        if (bind->argc == 0){
            return SRN_OK;
        } else {
            goto missing_arg;
        }
    }

    if (bind->argc == 0){
        goto too_many_arg;
    }

    if (bind->argc == COMMAND_ARB_ARGC){
        cmd->argv[0] = ptr;
        return SRN_OK;
    }

    int narg = 0;
    for (int i = 0; i < bind->argc - 1; i++){
        if (get_quote_arg(ptr, &cmd->argv[narg], &ptr) != SRN_OK){
            return SRN_ERR;
        }
        if (!ptr){
            cmd->argv[narg] = NULL;
            goto missing_arg;
        }
        narg++;
    }
    if (!ptr){
        cmd->argv[narg] = NULL;
        goto missing_arg;
    }
    get_last_quote_arg(ptr, &cmd->argv[narg]);

    /* Debug output */
    {
        DBG_FR("command: %s", cmd->bind->name);
        for (int i = 0; i < nopt; i++){
            DBG_FR("opt: '%s'", cmd->opt_key[i]);
            if (bind->opt_default_val[i] != NULL)
                DBG_FR("val: '%s'", cmd->opt_val[i]);
        }

        for (int i = 0; i < bind->argc; i++){
            DBG_FR("argv: '%s'", cmd->argv[i]);
        }
    }

    return SRN_OK;

    // Don't care
missing_arg:
    // TODO: it shoule be cared
    WARN_FR("Missing argument");
    return 0;

unknown_opt:
    WARN_FR("Unknown option '%s'", cmd->opt_key[nopt]);
    ctx->on_unknown_opt(cmd, cmd->opt_key[nopt], user_data);
    return -1;

too_many_opt:
    WARN_FR("Too many options");
    ctx->on_too_many_opt(cmd, user_data);
    return -1;

missing_opt_val:
    WARN_FR("Missing vaule for option '%s'", cmd->opt_key[nopt]);
    ctx->on_missing_opt_val(cmd, cmd->opt_key[nopt], user_data);
    return -1;

too_many_arg:
    WARN_FR("Too many arguments");
    ctx->on_too_many_arg(cmd, user_data);
    return -1;
}

/* inner test case */
void get_quote_arg_test() {
    char *start, *end;
    assert(get_quote_arg(strdup("'test'"), &start, &end) == 0);
    assert(strcmp(start, "test") == 0);
    assert(strcmp(end, "\0") == 0);

    assert(get_quote_arg(strdup("'test''123'"), &start, &end) == 0);
    assert(strcmp(start, "test") == 0);
    assert(strcmp(end, "'123'") == 0);

    assert(get_quote_arg(strdup("test   123 4"), &start, &end) == 0);
    assert(strcmp(start, "test") == 0);
    assert(strcmp(end, "123 4") == 0);

    assert(get_quote_arg(strdup("'test"), &start, &end) == -1);

    assert(get_quote_arg(strdup("'test\\'"), &start, &end) == -1);

    assert(get_quote_arg(strdup("'test\\\\''"), &start, &end) == 0);
    assert(strcmp(start, "test\\\\") == 0);
    assert(strcmp(end, "'") == 0);
}
