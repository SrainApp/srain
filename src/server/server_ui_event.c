#define __DBG_ON
#define __LOG_ON

#include <string.h>

#include "server.h"
#include "server_cmd.h"
#include "server_ui_event.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "rc.h"

static int get_server_and_chat(SuiSession *sui, Server **srv, Chat **chat);

void server_ui_event_activate(SuiEvent event, const char *params[], int count){
    rc_read();
    // TODO: welcome or command error report
}

void server_ui_event_connect(SuiEvent event, const char *params[], int count){
    Server *srv;

    g_return_if_fail(count == 9);
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
    g_return_if_fail(srv);

    server_connect(srv);
}

void server_ui_event_disconnect(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;

    g_return_if_fail(count == 0);
    g_return_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK);

    server_disconnect(srv);
    server_free(srv);
}

void server_ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *msg;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    msg = params[0];

    g_return_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK);

    // Command or message?
    if (msg[0] == '/'){
        if (server_cmd(srv, chat, msg) == SRN_OK){
            sui_add_sys_msgf(sui, SYS_MSG_NORMAL, 0,
                    _("Command \"%s\" finished"), msg);
        }
    } else {
        if (sirc_cmd_msg(srv->irc, chat->name, msg) == SRN_OK){
            sui_add_sent_msg(sui, msg, 0);
        } else {
            sui_add_sys_msgf(sui, SYS_MSG_ERROR, 0,
                    _("Failed to send message \"%s\""), msg);
        }
    }
}

void server_ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    const char *passwd;
    Server *srv;

    g_return_if_fail(count == 1 || count == 2);
    name = params[0];
    passwd = count == 2 ? params[1] : NULL;

    g_return_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK);

    sirc_cmd_join(srv->irc, name, passwd);
}

void server_ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 0);
    g_return_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK);

    if (chat->joined) {
        sirc_cmd_part(srv->irc, chat->name, "Leave.");
    } else {
        server_rm_chat(srv, chat->name);
    }
}

void server_ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *name;
    Server *srv;

    g_return_if_fail(count == 1);
    name = params[0];

    g_return_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK);

    server_add_chat(srv, name, NULL);
}

void server_ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count){
    Server *srv;
    Chat *chat;

    g_return_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK);

    server_rm_chat(srv, chat->name);
}

void server_ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    g_return_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK);

    sirc_cmd_kick(srv->irc, nick, chat->name, "Kick.");
}

void server_ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;
    Chat *chat;

    g_return_if_fail(count == 1);
    nick = params[0];

    g_return_if_fail(get_server_and_chat(sui, &srv, &chat) == SRN_OK);

    sirc_cmd_invite(srv->irc, nick, chat->name);
}

void server_ui_event_whois(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;
    Server *srv;

    g_return_if_fail(count == 1);
    nick = params[0];

    g_return_if_fail(get_server_and_chat(sui, &srv, NULL) == SRN_OK);

    sirc_cmd_whois(srv->irc, nick);
}

void server_ui_event_ignore(SuiSession *sui, SuiEvent event, const char *params[], int count){
    const char *nick;

    g_return_if_fail(count == 1);
    nick = params[0];

    // TODO: ignore filter
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
