#ifndef __IDLES_H
#define __IDLES_H

#include "irc.h"

void get_cur_time(char *timestr);
void idles_msg_normal(const irc_msg_t *imsg);
void idles_msg_server(const char *server_alias, const irc_msg_t *imsg);
void idles_join_part(const irc_msg_t *imsg);
void idles_topic(const irc_msg_t *imsg);
void idles_names(const irc_msg_t *imsg);

void add_idle_ui_msg_sys(const char *chan, const char *msg);

#endif /* __IDLES_H */
