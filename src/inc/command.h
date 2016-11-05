#ifndef __COMMAND_H
#define __COMMAND_H

#define COMMAND_MAX_OPTS 20
#define COMMAND_MAX_ARGS 20

typedef struct _SRVCommand SRVCommand;

typedef int (SRVCommandCallback) (SRVCommand *cmd, void *user_data);

typedef struct {
    char *name;
    unsigned argc;
    char *opt_key[COMMAND_MAX_OPTS];
    char *opt_default_val[COMMAND_MAX_OPTS];
    SRVCommandCallback *cb;
} SRVCommandBind;

struct _SRVCommand {
    char *argv[COMMAND_MAX_ARGS];
    char *opt_key[COMMAND_MAX_OPTS];
    char *opt_val[COMMAND_MAX_OPTS];

    SRVCommandBind *bind;
    char *rawcmd;
};

void commmad_bind(SRVCommandBind *binds);
int command_proc(const char *rawcmd, void *user_data);
const char *command_get_arg(SRVCommand *cmd, unsigned index);
int command_get_opt(SRVCommand *cmd, const char *opt_key, char **opt_val);

void command_test();

#endif /* __COMMAND_H */
