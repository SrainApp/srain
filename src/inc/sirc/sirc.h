#ifndef __IRC_H
#define __IRC_H

#include <glib.h>

typedef int SircSessionFlag;
typedef struct _SircSession SircSession;

#define SIRC_SESSION_SSL        1 << 0 // Not support yet
#define SIRC_SESSION_NOVERIFY   1 << 1 // Not support yet
#define SIRC_SESSION_SASL       1 << 2 // Not support yet

#define SIRC_BUF_LEN    513

#define __IN_SIRC_H
#include "sirc_cmd.h"
#include "sirc_event.h"
#include "sirc_numeric.h"
#include "sirc_utils.h"
#undef __IN_SIRC_H

SircSession* sirc_new_session(SircEvents *events, SircSessionFlag flag);
void sirc_free_session(SircSession *sirc);
void sirc_connect(SircSession *sirc, const char *host, int port);
void sirc_disconnect(SircSession *sirc);
int sirc_get_fd(SircSession *sirc);
SircSessionFlag sirc_get_flag(SircSession *sirc);
SircEvents* sirc_get_events(SircSession *sirc);
void* sirc_get_ctx(SircSession *sirc);
void sirc_set_ctx(SircSession *sirc, void *ctx);

#endif /* __IRC_H */
