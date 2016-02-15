#ifndef __IDLES_H
#define __IDLES_H

#include "irc.h"

void get_cur_time(char *timestr);
void add_idle_ui_msg_recv(const irc_msg_t *imsg);
void add_idle_ui_msg_recv2(const char *chan, const irc_msg_t *imsg);
void add_idle_ui_msg_recv3(const char *chan, const irc_msg_t *imsg);
void add_idle_ui_msg_sys(const irc_msg_t *imsg);
void add_idle_ui_chan_set_topic(const irc_msg_t *imsg);
void add_idles_ui_online_list_init(irc_msg_t *imsg);

#endif /* __IDLES_H */
