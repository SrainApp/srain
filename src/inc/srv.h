#ifndef __SRV_H
#define __SRV_H

/* Interface function pointers */
typedef int (*SRVJoinFunc) (const char *srv_name, const char *chan_name, const char *passwd);
typedef int (*SRVPartFunc) (const char *srv_name, const char *chan_name);
typedef int (*SRVSendFunc) (const char *srv_name, const char *target, const char *msg);
typedef int (*SRVCmdFunc) (const char *srv_name, const char *source, char *cmd, int block);

/* Macros for defining interface function */
#define DECLARE_SRVJoinFunc(func) int func (const char *srv_name, const char *chan_name, const char *passwd)
#define DECLARE_SRVPartFunc(func) int func (const char *srv_name, const char *chan_name);
#define DECLARE_SRVSendFunc(func) int func (const char *srv_name, const char *target, const char *msg)
#define DECLARE_SRVCmdFunc(func) int func (const char *srv_name, const char *source, char *cmd, int block)

void srv_init();
void srv_finalize();
int srv_connect(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname, int ssl);
int srv_quit(const char *srv_name, const char *reason);

/* SRV interface functions, used by other module */
DECLARE_SRVJoinFunc(srv_join);
DECLARE_SRVPartFunc(srv_part);
DECLARE_SRVSendFunc(srv_send);
DECLARE_SRVCmdFunc(srv_cmd);

#endif /* __SRV_H */
