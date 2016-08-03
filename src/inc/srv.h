#ifndef __SRV_H
#define __SRV_H

void srv_init();
int srv_connect(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname, int ssl);
int srv_send(const char *srv_name, const char *target, const char *msg);
int srv_cmd(const char *srv_name, const char *source, char *cmd, int block);
int srv_join(const char *srv_name, const char *chan, const char *passwd);
int srv_part(const char *srv_name, const char *chan, const char *reason);
int srv_quit(const char *srv_name, const char *reason);

#endif /* __SRV_H */
