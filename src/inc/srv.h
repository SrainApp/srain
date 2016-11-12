#ifndef __SRV_H
#define __SRV_H

/* Macros for defining interface function */
#define SIGN_SRV_JOIN const char *srv_name, const char *chan_name, const char *passwd
#define SIGN_SRV_PART const char *srv_name, const char *chan_name
#define SIGN_SRV_SEND const char *srv_name, const char *target, const char *msg
#define SIGN_SRV_CMD const char *srv_name, const char *source, char *cmd, int block


/* Interface function pointers */
typedef int (*SRVJoinFunc) (SIGN_SRV_JOIN);
typedef int (*SRVPartFunc) (SIGN_SRV_PART);
typedef int (*SRVSendFunc) (SIGN_SRV_SEND);
typedef int (*SRVCmdFunc)  (SIGN_SRV_CMD);

void srv_init();
void srv_finalize();
int srv_connect(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname, int ssl);
int srv_quit(const char *srv_name, const char *reason);

/* SRV interface functions, used by other module */
int srv_join (SIGN_SRV_JOIN);
int srv_part (SIGN_SRV_PART);
int srv_send (SIGN_SRV_SEND);
int srv_cmd  (SIGN_SRV_CMD);

#endif /* __SRV_H */
