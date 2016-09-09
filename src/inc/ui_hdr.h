#ifndef __UI_HDR_H
#define __UI_HDR_H

#include "srain_chat.h"

void ui_hdr_init();
int ui_hdr_srv_join(const char *chat_name, const char *passwd);
int ui_hdr_srv_part(SrainChat *chat);
int ui_hdr_srv_send(SrainChat *target, const char *msg);
int ui_hdr_srv_cmd(SrainChat *source, char *cmd, int block);

#endif /* __UI_INFT_H */
