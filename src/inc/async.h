#ifndef __ASYNC_H
#define __ASYNC_H

#include "irc.h"

/* async UI funcition, called in irc-listening thread ; */
void async_call_ui_msg_sys(const char *chan, const char *msg);
void async_online_list_init(irc_msg_t *imsg);
void async_join_part(const irc_msg_t *imsg);
void async_set_topic(const irc_msg_t *imsg);
void async_recv_msg_normal(irc_msg_t *imsg);
void async_recv_msg_server(const char *server_alias, const irc_msg_t *imsg);

#endif /* __ASYNC_H */
