#ifndef __SRV_EVENT_H
#define __SRV_EVENT_H

#include "libircclient.h"

void srv_event_connect(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_nick(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_quit(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_join(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_part(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_mode(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_umode(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_topic(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_kick(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_channel(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_privmsg(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_notice(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_channel_notice(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_invite(irc_session_t *session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_ctcp_action(irc_session_t *irc_session, const char *event,
        const char *origin, const char **params, unsigned int count);

void srv_event_numeric (irc_session_t *session, unsigned int event,
        const char *origin, const char **params, unsigned int count);

#endif /* __SRV_EVENT_H */
