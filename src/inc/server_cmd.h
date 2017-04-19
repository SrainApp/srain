#ifndef __SERVER_CMD_H
#define __SERVER_CMD_H

#include "server.h"

void server_cmd_init();
int server_cmd(Chat *chat, const char *cmd);

#endif /* __SERVER_CMD_H */
