#ifndef __IRC_H
#define __IRC_H

#include <glib.h>

#define SIRC_BUF_LEN 513

typedef void (*SircSimpleEventCallback) (const char *event, void *ctx);

typedef void (*SircEventCallback) (void *ctx, const char *event, const char *origin,
        const char **params, int count);

typedef void (*SircNumericEventCallback) (void *ctx, int event, const char *origin,
        const char **params, int count);

typedef struct {
    SircSimpleEventCallback     connect;
    SircSimpleEventCallback     disconnect;
    SircSimpleEventCallback     ping;

    SircNumericEventCallback    welcome;
    SircEventCallback           nick;
    SircEventCallback           quit;
    SircEventCallback           join;
    SircEventCallback           part;
    SircEventCallback           mode;
    SircEventCallback           umode;
    SircEventCallback           topic;
    SircEventCallback           kick;
    SircEventCallback           channel;
    SircEventCallback           privmsg;
    SircEventCallback           notice;
    SircEventCallback           channel_notice;
    SircEventCallback           invite;
    SircEventCallback           ctcp_req;
    SircEventCallback           ctcp_rep;
    SircEventCallback           ctcp_action;
    SircEventCallback           unknown;

    SircNumericEventCallback    numeric;
} SircEvents;

typedef struct {
    int fd;
    char buf[SIRC_BUF_LEN];
    GMutex mutex;  // Buffer lock

    SircEvents events; // Event callbacks
    void *ctx;
} SircSession;

int sirc_init();
SircSession* sirc_new(void *ctx);
void sirc_free(SircSession *sirc);
void sirc_connect(SircSession *sirc, const char *host, int port);
void sirc_disconnect(SircSession *sirc);

#endif /* __IRC_H */
