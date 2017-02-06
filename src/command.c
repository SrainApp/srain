/**
:A
 * @file command.c
 * @brief Flexible command parser
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-11-05
 */

// #define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "command.h"

#include "log.h"

static CommandContext *cmd_context = NULL;

static Command* command_alloc(CommandBind *bind, const char *rawcmd){
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

static int get_arg(char *ptr, char **start, char **end){
    *start = strtok(ptr, " ");
    *end = strtok(NULL, "");
    if (*end != NULL){
        *end = g_strchug(*end);  // Remove leading whitespace
    }

    if (*start == NULL){
        return -1;
    } else {
        return 0;
    }
}

static int get_quote_arg(char *ptr, char **start, char **end){
    int escape = 0;

    if (ptr == NULL || *ptr == '\0'){
        ERR_FR("Get a NULL or '\\0'");
        return -1;
    }

    ptr = g_strchug(ptr); // Remove leading whitespace

    if (*ptr != '\'') {
        return get_arg(ptr, start, end);
    }

    *ptr++ = '\0';
    *start = ptr;
    while (ptr++) {
        if (*ptr == '\0'){
            goto fail;
        }
        if (*ptr == '\\' && escape == 0){
            escape = 1;
            strcpy(ptr, ptr + 1);
            continue;
        }
        escape = 0;
        if (*ptr == '\''){
            *ptr++ = '\0';
            while (*ptr == ' ') *ptr++ = '\0';
            *end = ptr;
            return 0;
        }
    }

fail:
    ERR_FR("Unterminal quote");
    return -1;
}

static int command_parse(Command *cmd, void *user_data){
    unsigned nopt = 0;
    char *ptr, *tmp;
    CommandBind *bind = cmd->bind;

    get_arg(cmd->rawcmd, &tmp, &ptr);

    if (bind->argc == 0){
        if (ptr == NULL) {
            return 0;
        } else {
            goto too_many_arg;
        }
    }

    if (ptr == NULL){
        goto missing_arg;
    }

    while (ptr && *ptr == '-'){
        int i;
        for (i = 0; bind->opt_key[i] != NULL; i++){
            if (strncasecmp(bind->opt_key[i],
                        ptr,
                        strlen(bind->opt_key[i])) == 0){
                break;
            }
        }

        get_arg(ptr, &cmd->opt_key[nopt], &ptr);

        if (bind->opt_key[i] == NULL){
            goto unknown_opt;
        }
        // Option found

        DBG_FR("Option '%s' found", cmd->opt_key[nopt]);

        if (bind->opt_default_val[i] != NULL){
            if (ptr == NULL || *ptr == '-'){
                goto missing_opt_val;
            }
            // Get option vaule
            get_quote_arg(ptr, &cmd->opt_val[nopt], &ptr);
        } else {
            cmd->opt_val[nopt] = NULL;
        }

        if (nopt >= COMMAND_MAX_OPTS){
            goto too_many_opt;
        }

        nopt++;
    }
    cmd->opt_key[nopt] = NULL;

    for (int i = 0; i < bind->argc - 1; i++){
        if (get_quote_arg(ptr, &cmd->argv[i], &ptr) < 0){
            cmd->argv[i] = NULL;
            goto missing_arg;
        }
    }

    cmd->argv[bind->argc - 1] = ptr;
    if (ptr == NULL) goto missing_arg;

    for (int i = 0; i < nopt; i++){
        DBG_FR("opt: '%s'", cmd->opt_key[i]);
        if (bind->opt_default_val[i] != NULL)
            DBG_FR("val: '%s'", cmd->opt_val[i]);
    }

    for (int i = 0; i < bind->argc; i++){
        DBG_FR("argv: '%s'", cmd->argv[i]);
    }
    return 0;

    // Don't care
missing_arg:
    // WARN_FR("Missing argument");
    return 0;

unknown_opt:
    WARN_FR("Unknown option '%s'", cmd->opt_key[nopt]);
    cmd_context->on_unknown_opt(cmd, cmd->opt_key[nopt], user_data);
    return -1;

too_many_opt:
    WARN_FR("Too many options");
    cmd_context->on_too_many_opt(cmd, user_data);
    return -1;

missing_opt_val:
    WARN_FR("Missing vaule for option '%s'", cmd->opt_key[nopt]);
    cmd_context->on_missing_opt_val(cmd, cmd->opt_key[nopt], user_data);
    return -1;

too_many_arg:
    WARN_FR("Too many arguments");
    cmd_context->on_too_many_arg(cmd, user_data);
    return -1;
}

/**
 * @brief Let me know the name, arguments count, callback of
 *        specified commands, you should call this before using any command_*
 *        function
 *
 * @param ctx A CommandContext
 */
void commmad_set_context(CommandContext *ctx) {
    cmd_context = ctx;
}

/**
 * @brief Check and parse command, and call the corresponding callback function
 *
 * @param rawcmd A string of command
 * @param user_data Custom data passing to command callback function
 *
 * @return -1 if failed, the reason is various(no such command, no enouch
 *         argument, callback function failed, etc.)
 */
int command_proc(const char *rawcmd, void *user_data){
    int ret;
    Command *cmd;

    if (!cmd_context) return -1;

    for (int i = 0; cmd_context->binds[i].name != NULL; i++){
        if (strncasecmp(rawcmd, cmd_context->binds[i].name, strlen(cmd_context->binds[i].name)) == 0){
            cmd = command_alloc(&cmd_context->binds[i], rawcmd);
            if (command_parse(cmd, user_data) == 0){
                // callback
                ret = cmd->bind->cb(cmd, user_data);
                if (ret < 0){
                    cmd_context->on_callback_fail(cmd, user_data);
                }
            } else {
                ret = -1;
            }
            command_free(cmd);
            return ret;
        }
    }

    cmd_context->on_unknown_cmd(rawcmd, user_data);
    return -1;
}

/**
 * @brief Get the nth argument in the command
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
 * @brief Get the optional argument in the command
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
    int ret = 0;

    while (cmd->opt_key[i] != NULL){
        if (strcasecmp(cmd->opt_key[i], opt_key) == 0){
            ret = 1;
            if (opt_val != NULL){
                *opt_val = cmd->opt_val[i];
            }
            return ret;
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
            return ret;
        }
        i++;
    }

    ERR_FR("No such option '%s'", opt_key);
    return ret;
}

/**
 * Test case for commmand.c
 */

static void get_quote_arg_test() {
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
    assert(strcmp(start, "test\\") == 0);
    assert(strcmp(end, "'") == 0);
}

static int on_command_connect_test(Command *cmd, void *user_data){
    int testcase = (int)user_data;
    switch (testcase){
        case 1:
            {
                assert(strcmp(command_get_arg(cmd, 0), "127.0.0.1") == 0);
                assert(strcmp(command_get_arg(cmd, 1), "la") == 0);
                assert(command_get_arg(cmd, 2) == NULL);

                char *tmp;
                assert(command_get_opt(cmd, "-ssl", &tmp) == 1);
                assert(strcmp(tmp, "on") == 0);
                assert(command_get_opt(cmd, "-port", &tmp) == 1);
                assert(strcmp(tmp, "6667") == 0);
                assert(command_get_opt(cmd, "-nosuch", &tmp) == 0);
                break;
            }
        case 2:
            {
                assert(strcmp(command_get_arg(cmd, 0), "127.0.0.1 or irc.freenode.net") == 0);
                assert(strcmp(command_get_arg(cmd, 1), "la") == 0);
                assert(command_get_arg(cmd, 2) == NULL);

                char *tmp;
                assert(command_get_opt(cmd, "-ssl", &tmp) == 1);
                assert(strcmp(tmp, "-on") == 0);
                break;
            }
    }
    return 0;
}

static int on_command_topic_test(Command *cmd, void *user_data){
    int testcase = (int)user_data;
    switch (testcase){
        case 1:
            {
                assert(command_get_arg(cmd, 0) == NULL);

                char *tmp;
                assert(command_get_opt(cmd, "-del", &tmp) == 1);
                assert(tmp == NULL);
                break;
            }
        case 2:
            {
                assert(strcmp(command_get_arg(cmd, 0), "This is a topic") == 0);
                break;
            }
        case 3:
            {
                assert(strcmp(command_get_arg(cmd, 0), "'This is a topic'") == 0);
                break;
            }
        case 4:
            {
                assert(strcmp(command_get_arg(cmd, 0), "'-del'") == 0);
                break;
            }
    }
    return 0;
}

void on_anything_error(){
    LOG_FR("Some error occured");
}

static CommandBind cmd_binds_test[] = {
    {
        "/connect", 2,
        {"-port", "-ssl", "-passwd", "-realname", NULL},
        {"6667", "off", "", "Can you can a can?", NULL},
        on_command_connect_test
    }, {
        "/topic", 1,
        {"-del", NULL},
        {NULL, NULL},
        on_command_topic_test
    }, {
        NULL, 0, {NULL}, {NULL}, NULL
    },
};

static CommandContext context_test = {
    cmd_binds_test,
    on_anything_error,
    on_anything_error,
    on_anything_error,
    on_anything_error,
    on_anything_error,
    on_anything_error
};

void command_test(){
    get_quote_arg_test();

    commmad_set_context(&context_test);
    assert(command_proc("/connect -ssl  -on 127.0.0.1 la", 0) == -1);
    assert(command_proc("/connect -ssl on -pass 127.0.0.1 la", 0) == -1);
    assert(command_proc("/connect -ssl on 127.0.0.1 la", (void *)1) == 0);
    assert(command_proc("/connect -ssl '-on' '127.0.0.1 or irc.freenode.net' la", (void *)2) == 0);

    assert(command_proc("/topic", 0) == 0);
    assert(command_proc("/topic -del", (void *)1) == 0);
    assert(command_proc("/topic This is a topic", (void *)2) == 0);
    assert(command_proc("/topic 'This is a topic'", (void *)3) == 0);
    assert(command_proc("/topic '-del'", (void *)4) == 0);
}
