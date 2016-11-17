#ifndef __UI_HDR_H
#define __UI_HDR_H

#include "interfaces.h"

void ui_hdr_init();

SRVConnectFunc  ui_hdr_srv_connect;
SRVQueryFunc    ui_hdr_srv_query;
SRVUnqueryFunc  ui_hdr_srv_unquery;
SRVJoinFunc     ui_hdr_srv_join;
SRVPartFunc     ui_hdr_srv_part;
SRVQuitFunc     ui_hdr_srv_quit;
SRVSendFunc     ui_hdr_srv_send;
SRVCmdFunc      ui_hdr_srv_cmd;
SRVKickFunc     ui_hdr_srv_kick;
SRVWhoisFunc    ui_hdr_srv_whois;
SRVInviteFunc   ui_hdr_srv_invite;

#endif /* __UI_HDR_H */
