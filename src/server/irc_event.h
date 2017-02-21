#ifndef __IRC_EVENT_H
#define __IRC_EVENT_H

#include "sirc/sirc.h"

void irc_event_nick(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_quit(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_join(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_part(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_mode(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_umode(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_topic(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_kick(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_channel(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_privmsg(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_notice(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_channel_notice(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_invite(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_ctcp_action(SircSession *sirc, const char *event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_welcome(SircSession *sirc, int event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_numeric (SircSession *sirc, int event,
        const char *origin, const char *params[], int count, const char *msg);

void irc_event_connect(SircSession *sirc, const char *event);

void irc_event_disconnect(SircSession *sirc, const char *event);

void irc_event_ping(SircSession *sirc, const char *event);

#endif /* __IRC_EVENT_H */
