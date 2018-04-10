/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file app_ui_event.c
 * @brief Application UI event callbacks
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-07-19
 */

#include <string.h>

#include "core/core.h"
#include "sirc/sirc.h"
#include "config/reader.h"
#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "rc.h"
#include "filter.h"
#include "ret.h"
#include "utils.h"

static SrnServer* ctx_get_server(SuiBuffer *sui);
static SrnChat* ctx_get_chat(SuiBuffer *sui);

static SrnRet ui_event_open(SuiApplication *app, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_activate(SuiApplication *app, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_shutdown(SuiApplication *app, SuiEvent event, GVariantDict *params);

static SrnRet ui_event_connect(SuiWindow *win, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_server_list(SuiWindow *win, SuiEvent event, GVariantDict *params);

static SrnRet ui_event_disconnect(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_quit(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_send(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_join(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_part(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_query(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_unquery(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_kick(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_invite(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_whois(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_ignore(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_cutover(SuiBuffer *sui, SuiEvent event, GVariantDict *params);
static SrnRet ui_event_chan_list(SuiBuffer *sui, SuiEvent event, GVariantDict *params);

void srn_application_init_ui_event(SrnApplication *app){
    app->ui_app_events.open = ui_event_open;
    app->ui_app_events.activate = ui_event_activate;
    app->ui_app_events.shutdown = ui_event_shutdown;

    app->ui_win_events.connect = ui_event_connect;
    app->ui_win_events.server_list = ui_event_server_list;

    app->ui_events.disconnect = ui_event_disconnect;
    app->ui_events.quit = ui_event_quit;
    app->ui_events.send = ui_event_send;
    app->ui_events.join = ui_event_join;
    app->ui_events.part = ui_event_part;
    app->ui_events.query = ui_event_query;
    app->ui_events.unquery = ui_event_unquery;
    app->ui_events.kick = ui_event_kick;
    app->ui_events.invite = ui_event_invite;
    app->ui_events.whois = ui_event_whois;
    app->ui_events.ignore = ui_event_ignore;
    app->ui_events.cutover = ui_event_cutover;
    app->ui_events.chan_list = ui_event_chan_list;
}

static SrnRet ui_event_open(SuiApplication *app, SuiEvent event, GVariantDict *params){
    int len;
    char **urls;
    SrnApplication *srn_app;

    srn_app = sui_application_get_ctx(app);
    g_variant_dict_lookup(params, "urls", SUI_EVENT_PARAM_STRINGS, &urls);
    len = g_strv_length(urls);

    for (int i = 0; i < len; i ++){
        SrnRet ret;

        ret = srn_application_open_url(srn_app, urls[i]);
        if (!RET_IS_OK(ret)){
            return ret;
        }
    }

    return SRN_OK;
}

static SrnRet ui_event_activate(SuiApplication *app, SuiEvent event, GVariantDict *params){
    static bool activated = FALSE; // FIXME
    SrnApplication *srn_app;

    if (activated) {
        return SRN_OK;
    }

    activated = TRUE;
    srn_app = sui_application_get_ctx(app);
    sui_new_window(app, &srn_app->ui_win_events, sui_window_config_new());

    return rc_read(); // Read rc file
}

static SrnRet ui_event_shutdown(SuiApplication *app, SuiEvent event, GVariantDict *params){
    return SRN_OK;
}

static SrnRet ui_event_connect(SuiWindow *win, SuiEvent event, GVariantDict *params){
    const char *name = NULL;
    const char *nick = NULL;
    const char *realname= "";
    const char *username = "";
    SrnRet ret = SRN_ERR;
    SrnApplication *app;
    SrnServer *srv = NULL;
    SrnServerConfig *cfg = NULL;

    app = srn_application_get_default();

    g_variant_dict_lookup(params, "name", SUI_EVENT_PARAM_STRING, &name);
    if (!str_is_empty(name)){
        /* If params "name" is not specified, connecting to predefined server */
        cfg = srn_application_get_server_config(app, name);
        if (cfg->srv){
            return RET_ERR(_("Server \"%1$s\" already exists"), name);
        }
    } else {
        /* Else, it means that user is trying to connect to a custom server */
        char *addr;
        const char *host = NULL;
        int port = 0;
        const char *passwd = NULL;
        const char *encoding = "utf-8"; // TODO
        gboolean tls = FALSE;
        gboolean tls_noverify = FALSE;

        g_variant_dict_lookup(params, "host", SUI_EVENT_PARAM_STRING, &host);
        g_variant_dict_lookup(params, "port", SUI_EVENT_PARAM_INT, &port);
        g_variant_dict_lookup(params, "password", SUI_EVENT_PARAM_STRING, &passwd);
        g_variant_dict_lookup(params, "tls", SUI_EVENT_PARAM_BOOL, &tls);
        g_variant_dict_lookup(params, "tls-noverify", SUI_EVENT_PARAM_BOOL, &tls_noverify);

        /* Create server config */
        ret = srn_application_add_server_config(app, host);
        if (!RET_IS_OK(ret)){
            return RET_ERR(_("Failed to create server config: %1$s"), RET_MSG(ret));
        }
        cfg = srn_application_get_server_config(app, host);

        if (!str_is_empty(passwd)){
            str_assign(&cfg->passwd, passwd);
        }
        if (!str_is_empty(encoding)){
            str_assign(&cfg->irc->encoding, encoding);
        }
        cfg->irc->tls = tls;
        cfg->irc->tls_noverify = tls_noverify;

        addr = g_strdup_printf("%s:%d", host, port);
        srn_server_config_set_addr(cfg, addr);
        g_free(addr);
    }

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);
    g_variant_dict_lookup(params, "username", SUI_EVENT_PARAM_STRING, &username);
    g_variant_dict_lookup(params, "realname", SUI_EVENT_PARAM_STRING, &realname);

    if (!str_is_empty(nick)){
        str_assign(&cfg->nickname, nick);
    }
    if (!str_is_empty(username)){
        str_assign(&cfg->username, username);
    }
    if (!str_is_empty(realname)){
        str_assign(&cfg->realname, realname);
    }

    ret = srn_server_config_check(cfg);
    if (!RET_IS_OK(ret)){
        srn_application_rm_server_config(app, cfg);
        return ret;
    }
    ret = srn_application_add_server(app, cfg);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to instantiate server \"%1$s\""), cfg->name);
        srn_application_rm_server_config(app, cfg);
        return ret;
    }
    srv = srn_application_get_server(app, cfg->name);

    return srn_server_connect(srv);
}

static SrnRet ui_event_server_list(SuiWindow *win, SuiEvent event,
        GVariantDict *params){
    // FIXME: dirty hack
    GSList *lst;
    SrnApplication *app;

    app = srn_application_get_default();
    lst = app->srv_cfg_list;
    while (lst){
        SrnServerConfig *cfg = lst->data;
        if (cfg->predefined){
            sui_server_list_add(cfg->name);
        }
        lst = g_slist_next(lst);
    }

    return SRN_OK;
}

static SrnRet ui_event_disconnect(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    SrnRet ret;
    SrnServer *srv;
    SrnServerState prev_state;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    prev_state = srv->state;
    ret = srn_server_disconnect(srv);
    if (!RET_IS_OK(ret)){
        srn_chat_add_error_message(chat, chat->_user, RET_MSG(ret));
    }

    if (prev_state == SRN_SERVER_STATE_RECONNECTING){
        srn_chat_add_misc_message(chat, chat->_user, _("Reconnection stopped"));
    }

    return SRN_OK;
}

static SrnRet ui_event_quit(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    SrnRet ret;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    if (srn_server_is_registered(srv)) {
        ret = sirc_cmd_quit(srv->irc, srv->cfg->quit_message);
    } else {
        ret = srn_server_state_transfrom(srv, SRN_SERVER_ACTION_QUIT);
        if (!srn_server_is_valid(srv)){
            return SRN_OK;
        }
    }

    if (!RET_IS_OK(ret)){
        srn_chat_add_error_message(chat, chat->_user,
                _("Failed to quit from server, try again please"));
    }

    return ret;
}

static SrnRet ui_event_send(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *msg = NULL;
    SrnRet ret = SRN_ERR;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "message", SUI_EVENT_PARAM_STRING, &msg);
    g_return_val_if_fail(!str_is_empty(msg), SRN_ERR);

    // Command or message?
    if (msg[0] == '/'){
        ret = srn_application_run_command(srn_application_get_default(), msg);
        // FIXME: chat may be invalid now
        if (!srn_server_is_valid(srv)){
            return SRN_ERR;
        }
        if (RET_IS_OK(ret)){
            if (ret != SRN_OK) { // Has OK message
                srn_chat_add_misc_message(chat, chat->_user, RET_MSG(ret));
            }
        } else {
            srn_chat_add_error_message(chat, chat->_user, RET_MSG(ret));
        }
    } else {
        if (chat == chat->srv->chat) {
            ret = RET_ERR(_("Can not send message to a server"));
            srn_chat_add_error_message(chat, chat->_user, RET_MSG(ret));
            return ret;
        }

        srn_chat_add_sent_message(chat, msg); // Show on UI first

        ret = sirc_cmd_msg(chat->srv->irc, chat->name, msg);
        if (!RET_IS_OK(ret)){
            srn_chat_add_error_message_fmt(chat, chat->_user,
                    _("Failed to send message: %1$s"), RET_MSG(ret));
        }
    }

    return ret;
}

static SrnRet ui_event_join(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *chan = NULL;
    const char *passwd = NULL;
    SrnServer *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    g_variant_dict_lookup(params, "channel", SUI_EVENT_PARAM_STRING, &chan);
    g_variant_dict_lookup(params, "password", SUI_EVENT_PARAM_STRING, &passwd);

    return sirc_cmd_join(srv->irc, chan, passwd);
}

static SrnRet ui_event_part(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    if (chat->is_joined) {
        return sirc_cmd_part(srv->irc, chat->name, "Leave.");
    } else {
        return srn_server_rm_chat(srv, chat);
    }
}

static SrnRet ui_event_query(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    SrnServer *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return srn_server_add_chat(srv, nick);;
}

static SrnRet ui_event_unquery(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    return srn_server_rm_chat(srv, chat);
}

static SrnRet ui_event_kick(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return sirc_cmd_kick(srv->irc, nick, chat->name, "Kick.");
}

static SrnRet ui_event_invite(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(sui);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return sirc_cmd_invite(srv->irc, nick, chat->name);
}

static SrnRet ui_event_whois(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    SrnServer *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return sirc_cmd_whois(srv->irc, nick);
}

static SrnRet ui_event_ignore(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    const char *nick = NULL;
    SrnChat *chat;

    chat = ctx_get_chat(sui);
    g_return_val_if_fail(chat, SRN_ERR);

    g_variant_dict_lookup(params, "nick", SUI_EVENT_PARAM_STRING, &nick);

    return nick_filter_add_nick(chat, nick);
}

static SrnRet ui_event_cutover(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    SrnApplication *app;
    SrnServer *srv;
    SrnChat *chat;

    app = srn_application_get_default();
    srv = ctx_get_server(sui);
    g_return_val_if_fail(srn_application_is_server_valid(app, srv), SRN_ERR);
    chat = ctx_get_chat(sui);
    g_return_val_if_fail(chat, SRN_ERR);

    app->cur_srv = srv;
    srv->cur_chat = chat;

    return SRN_OK;
}

static SrnRet ui_event_chan_list(SuiBuffer *sui, SuiEvent event, GVariantDict *params){
    SrnServer *srv;

    srv = ctx_get_server(sui);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);

    return sirc_cmd_list(srv->irc, NULL, NULL);
}

/* Get a SrnServer object from SuiBuffer context (sui->ctx) */
static SrnServer* ctx_get_server(SuiBuffer *sui){
    SrnChat *chat;

    chat = sui_buffer_get_ctx(sui);
    g_return_val_if_fail(chat, NULL);

    return chat->srv;
}

/* Get a SrnChat object from SuiBuffer context (sui->ctx) */
static SrnChat* ctx_get_chat(SuiBuffer *sui){
    return sui_buffer_get_ctx(sui);
}
