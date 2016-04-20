#ifndef __SERVER_INTF_H
#define __SERVER_INTF_H

#include "srain_chan.h"

void server_intf_join(const char *chan_name);
void server_intf_part(SrainChan *chan);
void server_intf_send(SrainChan *target, const char *msg);
int server_intf_cmd(SrainChan *source, const char *cmd);

#endif /* __SERVER_INTF_H */
