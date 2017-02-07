#ifndef __SRV_EVENT_H
#define __SRV_EVENT_H

#include "srv.h"

void srv_event_welcome(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_nick(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_quit(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_join(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_part(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_mode(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_umode(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_topic(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_kick(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_channel(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_privmsg(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_notice(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_channel_notice(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_invite(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_ctcp_action(Server *srv, const char *event,
        const char *origin, const char **params, int count);

void srv_event_numeric (Server *srv, int event,
        const char *origin, const char **params, int count);

void srv_event_connect(const char *event, void *ctx);
void srv_event_disconnect(const char *event, void *ctx);
void srv_event_ping(const char *event, void *ctx);

#endif /* __SRV_EVENT_H */
