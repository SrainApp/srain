#ifndef __SRV_SESSION_CMD_H
#define __SRV_SESSION_CMD_H

#include "srv_session.h"

void srv_session_cmd_init();
int srv_session_cmd(SRVSession *session, const char *source, char *cmd, int block);

#endif /* __SRV_SESSION_CMD_H */
