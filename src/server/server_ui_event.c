#define __DBG_ON
#define __LOG_ON

#include <string.h>

#include "server.h"
#include "server_ui_event.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"

static int get_server_and_chat(SuiSession *sui, Server **srv, Chat **chat);

int server_ui_event_activate(SuiEvent event, const char *params[], int count){
    Server *srv = server_new("ngircd1", "127.0.0.1", 6667, "", FALSE, "UTF-8", "LA", NULL, NULL);
    server_connect(srv);
    Server *srv2 = server_new("ngircd2", "127.0.0.1", 6667, "", FALSE, "UTF-8", "CC", NULL, NULL);
    if (srv2) server_connect(srv2);

    return SRN_OK;
}

int server_ui_event_connect(SuiEvent event, const char *params[], int count){
    Server *srv;

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

int server_ui_event_disconnect(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;

    g_return_val_if_fail(count == 0, SRN_ERR);
    g_return_val_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK, SRN_ERR);

    server_disconnect(srv);
    server_free(srv);

    return SRN_OK;
}

int server_ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *msg;
    Server *srv;
    Chat *chat;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK, SRN_ERR);
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

int server_ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    const char *passwd;
    Server *srv;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK, SRN_ERR);
    g_return_val_if_fail(count == 1 || count == 2, SRN_ERR);

    name = params[0];
    passwd = count == 2 ? params[1] : NULL;

    return sirc_cmd_join(srv->irc, name, passwd);
}

int server_ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    Server *srv;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK, SRN_ERR);
    g_return_val_if_fail(count == 1, SRN_ERR);

    name = params[0];

    return sirc_cmd_part(srv->irc, name, "Leave.");
}

int server_ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    Server *srv;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK, SRN_ERR);
    g_return_val_if_fail(count == 1, SRN_ERR);

    name = params[0];

    return server_add_chat(srv, name, NULL);
}

int server_ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    Server *srv;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK, SRN_ERR);
    g_return_val_if_fail(count == 1, SRN_ERR);

    name = params[0];

    return server_rm_chat(srv, name);
}

int server_ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK, SRN_ERR);
    g_return_val_if_fail(count == 1, SRN_ERR);

    nick = params[0];

    return sirc_cmd_kick(srv->irc, nick, chat->name, "Kick.");
}

int server_ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK, SRN_ERR);
    g_return_val_if_fail(count == 1, SRN_ERR);

    nick = params[0];

    return sirc_cmd_invite(srv->irc, nick, chat->name);
}

int server_ui_event_whois(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;

    g_return_val_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK, SRN_ERR);

    g_return_val_if_fail(count == 1, SRN_ERR);
    nick = params[0];

    return sirc_cmd_whois(srv->irc, nick);
}

int server_ui_event_ignore(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;

    g_return_val_if_fail(count == 1, SRN_ERR);
    nick = params[0];

    // TODO: ignore filter
    return SRN_OK;
}

/* Get a Server object or Chat object from SuiSession context (sui->ctx),
 * if get a NULL value from context, SRN_ERR will be returned. */
static int get_server_and_chat(SuiSession *sui, Server **srv, Chat **chat){
    void *ctx;
    SuiSessionFlag flag;

    g_return_val_if_fail(sui, SRN_ERR);
    ctx = sui_get_ctx(sui);
    g_return_val_if_fail(ctx, SRN_ERR);

    flag = sui_get_flag(sui);

    if (flag & SUI_SESSION_SERVER){
        if (chat) *chat = NULL;
        if (srv) *srv = ctx;
    }
    else if (flag & SUI_SESSION_CHANNEL || flag & SUI_SESSION_DIALOG){
        if (chat) *chat = ctx;
        if (srv){
            *srv = ((Chat *)ctx)->srv;
            g_return_val_if_fail(*srv, SRN_ERR);
        }
    } else {
        ERR_FR("Unknow SuiSession type: %x", flag);
        return SRN_ERR;
    }

    return SRN_OK;
}
