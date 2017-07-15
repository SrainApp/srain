#ifndef __SERVER_CMD_H
#define __SERVER_CMD_H

#include "server.h"
#include "ret.h"

void server_cmd_init();
SrnRet server_cmd(Chat *chat, const char *cmd);

#endif /* __SERVER_CMD_H */
