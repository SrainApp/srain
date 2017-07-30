#ifndef __COMMAND_H
#define __COMMAND_H

#include "srain.h"
#include "ret.h"

#define COMMAND_MAX_OPTS        20
#define COMMAND_MAX_ARGS        20
#define COMMAND_MAX_SUBCMD      10

#define COMMAND_OPT_PREFIX      '-'
#define COMMAND_OPT_NO_VAL      NULL
#define COMMAND_OPT_NO_DEFAULT  (const char *) 1 // FIXME: a better way?

#define COMMAND_FLAG_OMIT_ARG   1 << 1

#define COMMAND_EMPTY_OPT { .key = NULL, .val = COMMAND_OPT_NO_VAL }
#define COMMAND_EMPTY {         \
    .name = NULL,               \
    .subcmd = {NULL},           \
    .argc = 0,                  \
    .opt = {COMMAND_EMPTY_OPT}, \
    .flag = 0,                  \
    .cb = NULL,                 \
    }

typedef int CommandFlag;
typedef struct _Command Command;
typedef struct _CommandBind CommandBind;
typedef struct _CommandOption CommandOption;
typedef struct _CommandContext CommandContext;
typedef SrnRet (CommandCallback) (Command *cmd, void *user_data);

struct _CommandOption {
    const char *key;
    const char *val;
};

struct _CommandBind {
    const char *name;
    const char *subcmd[COMMAND_MAX_SUBCMD];
    const int argc;
    const CommandOption opt[COMMAND_MAX_OPTS];
    const CommandFlag flag;
    CommandCallback *cb;
};

struct _CommandContext {
    CommandBind *binds;
};

SrnRet command_proc(CommandContext *ctx, const char *rawcmd, void *user_data);
const char *command_get_subcmd(Command *cmd);
const char *command_get_arg(Command *cmd, unsigned index);
bool command_get_opt(Command *cmd, const char *opt_key, const char **opt_val);

void command_test();
void get_quote_arg_test();

#endif /* __COMMAND_H */
