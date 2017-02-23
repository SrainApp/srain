#define __DBG_ON
#define __LOG_ON

#include <string.h>

#include "server.h"
#include "ui_event.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"

#define PRINT_EVENT_PARAM \
    do { \
        DBG_FR("sui: %p, event: %d", sui, event); \
        for (int i = 0; i < count; i++){ \
            if (i == 0) DBG_F("count: %d, params: [", count); \
            if (i == count - 1) { \
                DBG("%s]\n", params[i]); \
            } else { \
                DBG("%s ", params[i]); \
            } \
        } \
    } while (0)

#define PRINT_APP_EVENT_PARAM \
    do { \
        DBG_FR("event: %d", event); \
        for (int i = 0; i < count; i++){ \
            if (i == 0) DBG_F("count: %d, params: [", count); \
            if (i == count - 1) { \
                DBG("%s]\n", params[i]); \
            } else { \
                DBG("%s ", params[i]); \
            } \
        } \
    } while (0)

int ui_event_activate(SuiEvent event, const char *params[], int count){
    PRINT_APP_EVENT_PARAM;

    Server *srv = server_new("ngircd1", "127.0.0.1", 6667, "", FALSE, "UTF-8", "LA", NULL, NULL);
    server_connect(srv);
    Server *srv2 = server_new("ngircd2", "127.0.0.1", 6667, "", FALSE, "UTF-8", "CC", NULL, NULL);
    if (srv2) server_connect(srv2);

    return SRN_OK;
}

int ui_event_connect(SuiEvent event, const char *params[], int count){
    Server *srv;

    PRINT_APP_EVENT_PARAM;
    g_return_val_if_fail(count == 9, SRN_ERR);

    const char *name = params[0];
    const char *host = params[1];
    int port = atoi(params[2]);
    const char *passwd = params[3];
    bool ssl = strcmp(params[4], "TRUE") == 0 ? TRUE : FALSE;
    const char *encoding = params[5];
    const char *nick = params[6];
    const char *username = params[7];
    const char *realname= params[7];

    srv = server_new(name, host, port, passwd, ssl, encoding, nick, username, realname);
    if (!srv) {
        return SRN_ERR;
    }

    server_connect(srv);

    return SRN_OK;
}

int ui_event_disconnect(SuiSession *sui, SuiEvent event, const char *params[], int count){
    return SRN_OK;
}

int ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;
    const char *msg;
    SuiSessionFlag flag;

    flag = sui_get_flag(sui);
    if (flag & SUI_SESSION_SERVER){
        chat = NULL;
        srv = sui_get_ctx(sui);
    }
    else if (flag & SUI_SESSION_CHANNEL || flag & SUI_SESSION_DIALOG){
        chat = sui_get_ctx(sui);
        srv = chat->srv;
    } else {
        ERR_FR("Unknow SuiSession type: %x", flag);
        return SRN_ERR;
    }

    PRINT_EVENT_PARAM;

    g_return_val_if_fail(count == 1, SRN_ERR);
    msg = params[0];

    if (sirc_cmd_msg(srv->irc, chat->name, msg) == SRN_OK){
        sui_add_sent_msg(sui, msg, 0);
    } else {
        sui_add_sys_msgf(sui, SYS_MSG_ERROR, 0,
                _("Failed to send message \"%s\""), msg);
    }

    return SRN_OK;
}

int ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
