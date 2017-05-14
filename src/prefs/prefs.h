#ifndef __PREFS_H
#define __PREFS_H

#include "srain.h"
#include "server.h"

typedef struct _DefaultMessages DefaultMessages;
typedef struct _SuiAppPrefs     SuiAppPrefs;
typedef struct _SuiPrefs        SuiPrefs;
typedef struct _SircPrefs       SircPrefs;
typedef struct _ServerPrefs     ServerPrefs;

struct _DefaultMessages {
    const char *part_message;
    const char *kick_message;
    const char *away_message;
    const char *quit_message;
};

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

    DefaultMessages *def_msg;
};

struct _SircPrefs {
    bool auto_reconnect;
    bool use_ssl;
    bool notverify_cert;
    // bool use_ipv6;
    // bool use_sasl;
};

struct _ServerPrefs {
    User user;

    bool enable_log;
    bool strip_mirc_color;

    DefaultMessages *def_msg;
};

#endif /*__PREFS_H */
