#ifndef __SRAIN_H
#define __SRAIN_H

int srain_login(const char *name);
int srain_join(const char *chan);
int srain_send(const char *chan, const char *msg);
int srain_listen();

#endif /* __SRAIN_H */
