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
    bool auto_preview_image;
};

struct _SircPrefs {
    bool auto_reconnect;
    bool use_ssl;
    bool notverify_cert;
    // bool use_ipv6;
    // bool use_sasl;

    const char *part_msg;
    const char *kick_msg;
    const char *away_msg;
    const char *quit_msg;
};

struct _ServerPrefs {
    User user;

    bool enable_log;
    bool strip_mirc_color;
};

void prefs_init();
const char* prefs_read();
void prefs_finalize();

#endif /*__PREFS_H */
