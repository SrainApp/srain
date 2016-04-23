#ifndef __SERVER_CMD_H
#define __SERVER_CMD_H

#include "server.h"

int server_cmd(IRCServer *server, const char *chan_name, char *cmd);

#endif /* __SERVER_CMD_H */
