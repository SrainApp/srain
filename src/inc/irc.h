#ifndef __IRC_H
#define __IRC_H

#include "srv.h"

int irc_init();
void irc_connect(Server *srv);
void irc_disconnect(Server *srv);

#endif /* __IRC_H */
