#ifndef __UI_HDR_H
#define __UI_HDR_H

#include "srain_chan.h"

typedef int (*ServerJoinFunc) (const char *srv_name, const char *chan_name, const char *passwd);
typedef int (*ServerPartFunc) (const char *srv_name, const char *chan_name, const char *reason);
typedef int (*ServerSendFunc) (const char *srv_name, const char *target, const char *msg);
typedef int (*ServerCmdFunc) (const char *srv_name, const char *source, char *cmd);

int ui_hdr_srv_join(const char *chan_name, const char *passwd);
int ui_hdr_srv_part(SrainChan *chan, const char *reason);
int ui_hdr_srv_send(SrainChan *target, const char *msg);
int ui_hdr_srv_cmd(SrainChan *source, char *cmd);

void ui_hdr_init();

#endif /* __UI_INFT_H */
