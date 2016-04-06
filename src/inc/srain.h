#ifndef __SRAIN_H
#define __SRAIN_H

int srain_connect(const char *server, const char *alias);
int srain_login(const char *name);
int srain_query(const char *target);
int srain_unquery(const char *target);
int srain_join(const char *chan);
int srain_part(const char *chan, const char *reason);
int srain_cmd(const char *chan, char *cmd);
int srain_send(const char *chan, char *msg);
int srain_listen();
void srain_close();

#endif /* __SRAIN_H */
