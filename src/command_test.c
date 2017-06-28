/**
 * Test case for commmand.c
 */
#include <glib.h>
#include <assert.h>

#include "command.h"
#include "log.h"

static int on_command_connect_test(Command *cmd, void *user_data);
static int on_command_topic_test(Command *cmd, void *user_data);
static void on_anything_error();

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

    assert(command_proc(&context_test, "/connect -ssl  -on 127.0.0.1 la", 0) == -1);
    assert(command_proc(&context_test, "/connect -ssl on -pass 127.0.0.1 la", 0) == -1);
    assert(command_proc(&context_test, "/connect -ssl on 127.0.0.1 la", (void *)1) == 0);
    assert(command_proc(&context_test, "/connect -ssl '-on' '127.0.0.1 or irc.freenode.net' la", (void *)2) == 0);

    assert(command_proc(&context_test, "/topic", 0) == 0);
    assert(command_proc(&context_test, "/topic -del", (void *)1) == 0);
    assert(command_proc(&context_test, "/topic This is a topic", (void *)2) == 0);
    assert(command_proc(&context_test, "/topic 'This is a topic'", (void *)3) == 0);
    assert(command_proc(&context_test, "/topic '-del'", (void *)4) == 0);
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
