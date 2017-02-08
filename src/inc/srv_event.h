#ifndef __SRV_EVENT_H
#define __SRV_EVENT_H

#include "srv.h"

void srv_event_welcome(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_join(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_part(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char **params, int count);

void srv_event_numeric (SircSession *sirc, int event,
        const char *origin, const char **params, int count);

void srv_event_connect(SircSession *sirc, const char *event);
void srv_event_disconnect(SircSession *sirc, const char *event);
void srv_event_ping(SircSession *sirc, const char *event);

#endif /* __SRV_EVENT_H */
