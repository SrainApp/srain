#ifndef __UI_HDR_H
#define __UI_HDR_H

#include "srain_chan.h"

void ui_hdr_init();
int ui_hdr_srv_join(const char *chan_name, const char *passwd);
int ui_hdr_srv_part(SrainChan *chan, const char *reason);
int ui_hdr_srv_send(SrainChan *target, const char *msg);
int ui_hdr_srv_cmd(SrainChan *source, char *cmd, int block);

#endif /* __UI_INFT_H */
