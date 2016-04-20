#ifndef __UI_INFT_H
#define __UI_INFT_H

#include "srain_chan.h"

void ui_intf_server_join(const char *chan_name);
void ui_intf_server_part(SrainChan *chan);
void ui_intf_server_send(SrainChan *target, const char *msg);
int ui_intf_server_cmd(SrainChan *source, const char *cmd);

#endif /* __UI_INFT_H */
