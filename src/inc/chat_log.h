#ifndef __CHAT_LOG_H
#define __CHAT_LOG_H

void chat_log_log(const char *srv_name, const char *chan_name, const char *msg);
void chat_log_fmt(const char *srv_name, const char *chan_name, const char *fmt, ...);

#endif /* __CHAT_LOG_H */
