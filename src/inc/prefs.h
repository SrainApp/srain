#ifndef __PREFS_H
#define __PREFS_H

#include "srain.h"
#include "server.h"

typedef struct _SuiAppPrefs     SuiAppPrefs;
typedef struct _SuiPrefs        SuiPrefs;
typedef struct _SircPrefs       SircPrefs;
typedef struct _ServerPrefs     ServerPrefs;

struct _SuiAppPrefs {
    bool auto_switch_to_new_chat;
    // const char *font;
};

struct _SuiPrefs {
    bool notify;
    bool show_topic;
    bool show_avatar;
    bool show_user_list;
    bool send_by_ctrl_enter;
    bool preview_image;
    bool enable_log;
};

struct _SircPrefs {
    bool auto_reconnect;
    bool use_ssl;
    bool verify_ssl_cert;
    // bool use_ipv6;
    // bool use_sasl;
};

struct _ServerPrefs {
    /* For specificed server */
    const char *name;
    const char *host;
    int port;
    const char *passwd;
    const char *encoding;

    /* User */
    const char *nickname;
    const char *username;
    const char *realname;

    /* Default message */
    const char *part_message;
    const char *kick_message;
    const char *away_message;
    const char *quit_message;

    SircPrefs irc;
};

void prefs_init();
const char* prefs_read();
void prefs_finalize();

#endif /*__PREFS_H */
