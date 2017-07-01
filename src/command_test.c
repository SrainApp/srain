/**
 * Test case for commmand.c
 */
#include <glib.h>
#include <assert.h>
#include <string.h>

#include "srain.h"
#include "command.h"
#include "log.h"

static int on_command_connect_test(Command *cmd, void *user_data);
static int on_command_topic_test(Command *cmd, void *user_data);
static void on_anything_error();

static CommandBind cmd_binds_test[] = {
    {
        .name = "/connect",
        .subcmd = {NULL},
        .argc = 2,
        .opt_key = {"-port", "-ssl", "-passwd", "-realname", NULL},
        .opt_default_val = {"6667", "off", "", "Can you can a can?", NULL},
        .flag = 0,
        .cb = on_command_connect_test
    },
    {
        .name = "/topic",
        .subcmd = {"rm"},
        .argc = 1,
        .opt_key = {"-del", NULL},
        .opt_default_val = {NULL, NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_topic_test
    },
    COMMAND_EMPTY,
};

static CommandContext context_test = {
    .binds = cmd_binds_test,
    .on_unknown_cmd = on_anything_error,
    .on_unknown_opt = on_anything_error,
    .on_missing_opt_val = on_anything_error,
    .on_missing_arg = on_anything_error,
    .on_too_many_opt = on_anything_error,
    .on_too_many_arg = on_anything_error,
    .on_callback_fail = on_anything_error,
};

void command_test(){
    get_quote_arg_test();

    assert(command_proc(&context_test, "/connect -ssl  -on 127.0.0.1 la", 0) == SRN_ERR);
    assert(command_proc(&context_test, "/connect -ssl on -pass 127.0.0.1 la", 0) == SRN_ERR);
    assert(command_proc(&context_test, "/connect -ssl on 127.0.0.1 la", (void *)1) == SRN_OK);
    assert(command_proc(&context_test, "/connect -ssl '-on' '127.0.0.1 or irc.freenode.net' la", (void *)2) == SRN_OK);

    assert(command_proc(&context_test, "/topic", 0) == SRN_OK);
    assert(command_proc(&context_test, "/topic -del", (void *)1) == SRN_OK);
    assert(command_proc(&context_test, "/topic This is a topic", (void *)2) == SRN_OK);
    assert(command_proc(&context_test, "/topic 'This is a topic'", (void *)3) == SRN_OK);
    assert(command_proc(&context_test, "/topic '-del'", (void *)4) == SRN_OK);
    assert(command_proc(&context_test, "/topic rm", 5) == SRN_OK);
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
                assert(command_get_opt(cmd, "-ssl", &tmp));
                assert(strcmp(tmp, "on") == 0);
                assert(command_get_opt(cmd, "-port", &tmp));
                assert(strcmp(tmp, "6667") == 0);
                assert(!command_get_opt(cmd, "-nosuch", &tmp));
                break;
            }
        case 2:
            {
                assert(strcmp(command_get_arg(cmd, 0), "127.0.0.1 or irc.freenode.net") == 0);
                assert(strcmp(command_get_arg(cmd, 1), "la") == 0);
                assert(command_get_arg(cmd, 2) == NULL);

                char *tmp;
                assert(command_get_opt(cmd, "-ssl", &tmp));
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
                assert(command_get_opt(cmd, "-del", &tmp));
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
                LOG_FR("%s", command_get_arg(cmd, 0));
                assert(strcmp(command_get_arg(cmd, 0), "This is a topic") == 0);
                break;
            }
        case 4:
            {
                assert(strcmp(command_get_arg(cmd, 0), "-del") == 0);
                break;
            }
    }
    return 0;
}

static void on_anything_error(){
    LOG_FR("Some error occured");
}
