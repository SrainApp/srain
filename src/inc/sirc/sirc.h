#ifndef __IRC_H
#define __IRC_H

#include <glib.h>
#include <gio/gio.h>

typedef int SircSessionFlag;
typedef struct _SircSession SircSession;
typedef struct _SircPrefs SircPrefs;

#define SIRC_SESSION_SSL            1 << 0
#define SIRC_SESSION_SSL_NOTVERIFY  1 << 1
#define SIRC_SESSION_SASL           1 << 2 // Not support yet
#define SIRC_SESSION_IPV6           1 << 3 // Not support yet

#define SIRC_BUF_LEN    513

struct _SircPrefs {
    bool auto_reconnect;
    bool use_ssl;
    bool verify_ssl_cert;
    // bool use_ipv6;
    // bool use_sasl;
};

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
GIOStream* sirc_get_stream(SircSession *sirc);
SircSessionFlag sirc_get_flag(SircSession *sirc);
SircEvents* sirc_get_events(SircSession *sirc);
void* sirc_get_ctx(SircSession *sirc);
void sirc_set_ctx(SircSession *sirc, void *ctx);

#endif /* __IRC_H */
