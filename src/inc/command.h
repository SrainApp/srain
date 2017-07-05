#ifndef __COMMAND_H
#define __COMMAND_H

#include "srain.h"

#define COMMAND_MAX_OPTS        20
#define COMMAND_MAX_ARGS        20
#define COMMAND_OPT_PREFIX      '-'
#define COMMAND_MAX_SUBCMD      10

#define COMMAND_FLAG_OMIT_ARG   1 << 1

#define COMMAND_EMPTY       {   \
    .name = NULL,               \
    .subcmd = {NULL},           \
    .argc = 0,                  \
    .opt_key = {NULL},          \
    .opt_default_val = {NULL},  \
    .flag = 0,                 \
    .cb = NULL,                 \
    }

typedef int CommandFlag;
typedef struct _Command Command;

typedef int (CommandCallback) (Command *cmd, void *user_data);

typedef struct {
    char *name;
    char *subcmd[COMMAND_MAX_SUBCMD];
    int argc;
    char *opt_key[COMMAND_MAX_OPTS];
    char *opt_default_val[COMMAND_MAX_OPTS];
    CommandFlag flag;
    CommandCallback *cb;
} CommandBind;

struct _Command {
    char *subcmd;
    char *argv[COMMAND_MAX_ARGS];
    char *opt_key[COMMAND_MAX_OPTS];
    char *opt_val[COMMAND_MAX_OPTS];

    CommandBind *bind;
    char *rawcmd;
};

typedef struct {
    CommandBind *binds;
    void (*on_unknown_cmd) (const char *cmd, void *user_data);
    void (*on_unknown_opt) (Command *cmd, const char *opt, void *user_data);
    void (*on_missing_opt_val) (Command *cmd, const char *opt, void *user_data);
    void (*on_missing_arg) (Command *cmd, int narg, void *user_data);
    void (*on_too_many_opt) (Command *cmd, void *user_data);
    void (*on_too_many_arg) (Command *cmd, void *user_data);
    void (*on_callback_fail) (Command *cmd, void *user_data);
} CommandContext;

int command_proc(CommandContext *ctx, const char *rawcmd, void *user_data);
const char *command_get_arg(Command *cmd, unsigned index);
bool command_get_opt(Command *cmd, const char *opt_key, const char **opt_val);

void command_test();
void get_quote_arg_test();

#endif /* __COMMAND_H */
