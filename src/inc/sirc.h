#ifndef __IRC_H
#define __IRC_H

#include <glib.h>

#define CHAN_PREFIX1 '#'
#define CHAN_PREFIX2 '&'

#define SIRC_BUF_LEN 513

#define SIRC_IS_CHAN(ch) (ch && (ch[0] == CHAN_PREFIX1 || ch[0] == CHAN_PREFIX2))

typedef struct _SircSession SircSession;

#define __IN_SIRC_H
#include "sirc_event.h"
#undef __IN_SIRC_H

SircSession* sirc_new_session(SircEvents *events);
void sirc_free_session(SircSession *sirc);
void sirc_connect(SircSession *sirc, const char *host, int port);
void sirc_disconnect(SircSession *sirc);
int sirc_get_fd(SircSession *sirc);
SircEvents* sirc_get_events(SircSession *sirc);
void* sirc_get_ctx(SircSession *sirc);
void sirc_set_ctx(SircSession *sirc, void *ctx);

#endif /* __IRC_H */
