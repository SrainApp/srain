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
 * @file chat_command.c
 * @brief Command definitions and callbacks
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-07-26
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <glib.h>

#include "core/core.h"
#include "sui/sui.h"
#include "config/reader.h"
#include "srain.h"
#include "log.h"
#include "meta.h"
#include "i18n.h"
#include "command.h"
#include "render/render.h"
#include "filter/filter.h"
#include "utils.h"
#include "pattern_set.h"
#include "chat_command.h"

typedef struct _SrnChatCommandContext {
    SrnApplication *app;
    SrnServer *srv;
    SrnChat *chat;
} SrnChatCommandContext;

static SrnApplication* ctx_get_app(SrnChatCommandContext *cctx);
static SrnServer* ctx_get_server(SrnChatCommandContext *cctx);
static SrnChat* ctx_get_chat(SrnChatCommandContext *cctx);

/*******************************************************************************
 * Exported functions
 ******************************************************************************/

/**
 * @brief Run a command in context of SrnChat
 *
 * @param chat A SrnChat instance
 * @param cmd
 *
 * @return SRN_OK or SRN_ERR or other error
 */
SrnRet srn_chat_run_command(SrnChat *chat, const char *cmd){
    SrnCommandContext *ctx;
    SrnChatCommandContext cctx;

    g_return_val_if_fail(chat, SRN_ERR);
    g_return_val_if_fail(cmd, SRN_ERR);

    ctx = srn_application_get_default()->cmd_ctx;
    cctx.app = srn_application_get_default();
    cctx.srv = chat->srv;
    cctx.chat = chat;

    return srn_command_context_proc(ctx, cmd, &cctx);
}

/**
 * @brief Simplest commmand complete function
 *
 * @param chat
 * @param cmd
 *
 * @return A list of completed commands
 *
 * TODO: improve it.
 *
 */
GList* srn_chat_complete_command(SrnChat *chat, const char *cmd){
    int i;
    char *lower_cmd;
    GList *lst;

    if (!g_str_has_prefix(cmd, "/")){
        return NULL;
    }

    i = 0;
    lower_cmd = g_utf8_strdown(cmd, -1);
    lst = NULL;
    while (cmd_bindings[i].name){
        if (g_str_has_prefix(cmd_bindings[i].name, lower_cmd)){
            lst = g_list_append(lst, g_strdup(cmd_bindings[i].name));
        }
        for (int j = 0; cmd_bindings[i].alias[j]; j++) {
            if (g_str_has_prefix(cmd_bindings[i].alias[j], lower_cmd)){
                lst = g_list_append(lst, g_strdup(cmd_bindings[i].alias[j]));
            }
        }
        i++;
    }
    g_free(lower_cmd);

    return lst;
}

/*******************************************************************************
 * SrnCommand callbacks
 ******************************************************************************/

SrnRet on_command_reload(SrnCommand *cmd, void *user_data){
    SrnApplication *app;

    app = ctx_get_app(user_data);

    return srn_application_reload_config(app);
}

SrnRet on_command_server(SrnCommand *cmd, void *user_data){
    const char *subcmd;
    const char *name;
    SrnRet ret;
    SrnApplication *app;
    SrnServer *srv;

    app = ctx_get_app(user_data);
    subcmd = srn_command_get_subcmd(cmd);
    g_return_val_if_fail(subcmd, SRN_ERR);

    name = srn_command_get_arg(cmd, 0);

    if (g_ascii_strcasecmp(subcmd, "connect") == 0){
        srv = srn_application_get_server(app, name);
        if (!srv) { // Create one
            ret = srn_application_add_server(app, name);
            if (!RET_IS_OK(ret)){
                return ret;
            }
            srv = srn_application_get_server(app, name);
        }
        if (!srv){ // Still fail
            return RET_ERR(_("Failed to instantiate server \"%1$s\""), name);
        }

        ret = srn_server_connect(srv);
        if (!RET_IS_OK(ret)) {
            return ret;
        }

        srn_server_wait_until_registered(srv); // FIXME: busy wait
        if (!srn_server_is_registered(srv)){
            return RET_ERR(_("Failed to register on server \"%1$s\""), name);
        }

        return SRN_OK;
    }

    if (g_ascii_strcasecmp(subcmd, "disconnect") == 0){
        SrnServerState prev_state;

        srv = srn_application_get_server(app, name);
        if (!srv) {
            // FIXME: better errmsg?
            return RET_ERR(_("Cannot disconnect from a unconnected server"));
        }

        // FIXME: looks tricky
        prev_state = srv->state;
        ret = srn_server_disconnect(srv);
        if (RET_IS_OK(ret) && prev_state == SRN_SERVER_STATE_RECONNECTING){
            ret = RET_OK(_("Reconnection stopped"));
        }

        return ret;
    }

    if (g_ascii_strcasecmp(subcmd, "list") == 0){
        GList *lst;
        GList *srv_cfg_lst;
        GString *str;

        // Get a list of server name
        srv_cfg_lst = NULL;
        ret = srn_config_manager_read_server_config_list(
                app->cfg_mgr, &srv_cfg_lst);
        if (!RET_IS_OK(ret)){
            return ret;
        }

        str = g_string_new(NULL);
        for (lst = srv_cfg_lst; lst; lst = g_list_next(lst)) {
            g_string_append_c(str, ' ');
            g_string_append(str, lst->data);
        }
        ret = RET_OK(_("%1$d available server(s):%2$s"),
                g_list_length(srv_cfg_lst), str->str);
        g_string_free(str, TRUE);
        g_list_free_full(srv_cfg_lst, g_free);

        return ret;
    }

    return RET_ERR(_("Unknown sub command: %1$s"), subcmd);
}

SrnRet on_command_connect(SrnCommand *cmd, void *user_data){
    const char *name;
    const char *addr_str;
    const char *passwd;
    const char *nick;
    const char *user;
    const char *real;
    const char *encoding;
    SrnRet ret = SRN_OK;
    SrnApplication *app;
    SrnServer *srv;
    SrnServerConfig *cfg;
    SrnServerAddr *addr;

    app = ctx_get_app(user_data);
    addr_str = srn_command_get_arg(cmd, 0);
    if (!addr_str) {
        return RET_ERR(_("Missing argument <addr>"));
    }
    nick = srn_command_get_arg(cmd, 1);

    srv = NULL;
    cfg = srn_server_config_new();
    addr = srn_server_addr_new_from_string(addr_str);

    // Try looking for server config with the same address in config
    ret = srn_config_manager_read_server_config_by_addr(app->cfg_mgr, cfg, addr);
    if (!RET_IS_OK(ret)){
        // If not found, just load a default server config from config
        ret = srn_config_manager_read_server_config(app->cfg_mgr, cfg, NULL);
        if (!RET_IS_OK(ret)){
            goto FIN;
        }
        // Add address for default server config
        (void)srn_server_config_add_addr(cfg, addr);
        addr = NULL; // Ownership changed to server config
    }

    if (!cfg->addrs){
        // This should not happend
        ret = SRN_ERR;
        g_warn_if_reached();
        goto FIN;
    }
    // Use first address of config address list as server name
    name = ((SrnServerAddr*)cfg->addrs->data)->host;

    if (srn_command_get_opt(cmd, "-pwd", &passwd)){
        str_assign(&cfg->password, passwd);
    }

    if (nick){
        str_assign(&cfg->user->nick, nick);
    }
    if (srn_command_get_opt(cmd, "-user", &user)){
        str_assign(&cfg->user->username, user);
    }
    if (srn_command_get_opt(cmd, "-real", &real)){
        str_assign(&cfg->user->realname, real);
    }

    if (srn_command_get_opt(cmd, "-encode", &encoding)){
        str_assign(&cfg->irc->encoding, encoding);
    }
    cfg->irc->tls_noverify = srn_command_get_opt(cmd, "-tls-noverify", NULL);
    cfg->irc->tls = srn_command_get_opt(cmd, "-tls", NULL)
        || cfg->irc->tls_noverify;

    ret = srn_application_add_server_with_config(app, name, cfg);
    if (!RET_IS_OK(ret)) {
        ret = RET_ERR(_("Failed to instantiate server \"%1$s\""), name);
        goto FIN;
    }
    cfg = NULL; // Ownership changed to server
    srv = srn_application_get_server(app, name);

    ret = srn_server_connect(srv);
    if (!RET_IS_OK(ret)){
        goto FIN;
    }

    srn_server_wait_until_registered(srv);
    if (!srn_server_is_registered(srv)){
        ret = RET_ERR(_("Failed to register on server \"%1$s\""), srv->name);
        goto FIN;
    }

    ret = SRN_OK;
FIN:
    if (addr){
        srn_server_addr_free(addr);
    }
    if (cfg){
        srn_server_config_free(cfg);
    }
    if (!RET_IS_OK(ret) && srv){
        srn_application_rm_server(app, srv);
    }

    return ret;
}

SrnRet on_command_ignore(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *user;

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }
    g_return_val_if_fail(chat, SRN_ERR);

    user = srn_server_get_user(chat->srv, nick);
    if(!user){
        user = srn_server_add_and_get_user(chat->srv, nick);
    }

    if(user->is_ignored){
        return RET_ERR(_("\"%1$s\" is already ignored"), nick);
    }
    srn_server_user_set_is_ignored(user, TRUE);
    return RET_OK(_("\"%1$s\" has ignored"), nick);
}

SrnRet on_command_unignore(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *user;

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    user = srn_server_get_user(chat->srv, nick);
    if(!user){
        return RET_ERR(_("user \"%1$s\" not found"), nick);
    }
    srn_server_user_set_is_ignored(user, FALSE);
    return RET_OK(_("\"%1$s\" has unignored"), nick);
}

SrnRet on_command_filter(SrnCommand *cmd, void *user_data){
    const char *pattern;
    SrnRet ret;
    SrnChat *chat;
    SrnPatternSet *pattern_set;

    pattern = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(pattern, SRN_ERR);
    pattern_set = srn_application_get_default()->pattern_set;
    g_return_val_if_fail(pattern_set, SRN_ERR);

    if (srn_pattern_set_get(pattern_set, pattern) == NULL) {
        return RET_ERR(_("Pattern \"%1$s\" not found"), pattern);
    }

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        SrnServer *srv;

        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }
    g_return_val_if_fail(chat, SRN_ERR);

    ret = srn_filter_attach_pattern(chat->extra_data, pattern);
    if (!RET_IS_OK(ret)) {
        return RET_ERR(_("Failed to attach pattern to chat \"%1$s\": %2$s"),
                chat->name, RET_MSG(ret));
    }

    return RET_OK(_("Messages of chat \"%1$s\" will be filtered by pattern \"%2$s\""),
            chat->name, pattern);
}

SrnRet on_command_unfilter(SrnCommand *cmd, void *user_data){
    const char *pattern;
    SrnRet ret;
    SrnChat *chat;

    pattern = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(pattern, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        SrnServer *srv;

        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }
    g_return_val_if_fail(chat, SRN_ERR);

    ret = srn_filter_detach_pattern(chat->extra_data, pattern);
    if (!RET_IS_OK(ret)) {
        return RET_ERR(_("Failed to dettach pattern from chat \"%1$s\": %2$s"),
                chat->name, RET_MSG(ret));
    }

    return RET_OK(_("Messages of chat \"%1$s\" are no longer filtered by pattern \"%2$s\""),
            chat->name, pattern);
}

SrnRet on_command_query(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    // If chat already exists, activate its buffer.
    chat = srn_server_add_and_get_chat(srv, nick);
    if (chat) {
        sui_activate_buffer(chat->ui);
        return SRN_OK;
    }

    return srn_server_add_chat(srv, nick);
}

SrnRet on_command_unquery(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = ctx_get_chat(user_data);

    nick = srn_command_get_arg(cmd, 0);
    if (nick) {
        chat = srn_server_get_chat(srv, nick);
    }
    g_return_val_if_fail(chat, SRN_ERR);
    g_return_val_if_fail(chat->type == SRN_CHAT_TYPE_DIALOG, SRN_ERR);

    return srn_server_rm_chat(srv, chat);
}

SrnRet on_command_join(SrnCommand *cmd, void *user_data){
    const char *chan;
    const char *passwd;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    chan = srn_command_get_arg(cmd, 0);
    passwd = srn_command_get_arg(cmd, 1);

    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_join(srv->irc, chan, passwd);
}

SrnRet on_command_part(SrnCommand *cmd, void *user_data){
    const char *chan;
    const char *reason;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = ctx_get_chat(user_data);

    chan = srn_command_get_arg(cmd, 0);
    reason = srn_command_get_arg(cmd, 1);

    if (!chan && chat){
        chan = chat->name;
    }
    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_part(srv->irc, chan, reason);
}

SrnRet on_command_quit(SrnCommand *cmd, void *user_data){
    const char *reason;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    reason = srn_command_get_arg(cmd, 0);
    if (!reason) reason = PACKAGE_NAME " " PACKAGE_VERSION " " "quit.";

    return srn_server_quit(srv, reason);
}

SrnRet on_command_topic(SrnCommand *cmd, void *user_data){
    const char *topic;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = ctx_get_chat(user_data);
    g_return_val_if_fail(chat, SRN_ERR);

    /* Command "/topic" has flag SRN_COMMAND_FLAG_OMIT_ARG, we should deal with
     * argument count byself. */
    topic = srn_command_get_arg(cmd, 0);
    if (topic == NULL && srn_command_get_opt(cmd, "-rm", NULL)) {
        topic = "";
    }

    return sirc_cmd_topic(srv->irc, chat->name, topic);
}

SrnRet on_command_msg(SrnCommand *cmd, void *user_data){
    const char *target;
    const char *msg;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    msg = srn_command_get_arg(cmd, 1);
    target = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(msg, SRN_ERR);
    g_return_val_if_fail(target, SRN_ERR);

    if (sirc_cmd_msg(srv->irc, target, msg) == SRN_OK){
        return RET_OK(_("A message has been sent to \"%1$s\""), target);
    } else {
        return SRN_ERR;
    }
}

SrnRet on_command_me(SrnCommand *cmd, void *user_data){
    const char *msg;
    SrnRet ret;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srn_server_is_valid(srv), SRN_ERR);
    chat = ctx_get_chat(user_data);
    g_return_val_if_fail(chat, SRN_ERR);
    msg = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(msg, SRN_ERR);

    if (chat == chat->srv->chat) {
        return RET_ERR(_("Cannot send message directly to a server"));
    }

    ret = sirc_cmd_action(chat->srv->irc, chat->name, msg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Failed to send action message: %1$s"), RET_MSG(ret));
    }

    g_autoptr(SircMessageContext) context = sirc_message_context_new(NULL);

    srn_chat_add_action_message(chat, chat->user, msg, context);

    return SRN_OK;
}

SrnRet on_command_nick(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_nick(srv->irc, nick);
}

SrnRet on_command_whois(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_whois(srv->irc, nick);
}

SrnRet on_command_invite(SrnCommand *cmd, void *user_data){
    const char *nick;
    const char *chan;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = ctx_get_chat(user_data);

    nick = srn_command_get_arg(cmd, 0);
    chan = srn_command_get_arg(cmd, 1);
    g_return_val_if_fail(nick, SRN_ERR);

    if (!chan && chat) {
        chan = chat->name;
    }
    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_invite(srv->irc, nick, chan);
}

SrnRet on_command_kick(SrnCommand *cmd, void *user_data){
    const char *nick;
    const char *chan;
    const char *reason;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = ctx_get_chat(user_data);

    nick = srn_command_get_arg(cmd, 0);
    chan = srn_command_get_arg(cmd, 1);
    reason = srn_command_get_arg(cmd, 2);

    g_return_val_if_fail(nick, SRN_ERR);

    if (!chan && chat) {
        chan = chat->name;
    }

    return sirc_cmd_kick(srv->irc, nick, chan, reason);
}

SrnRet on_command_mode(SrnCommand *cmd, void *user_data){
    const char *mode;
    const char *target;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    target = srn_command_get_arg(cmd, 0);
    mode = srn_command_get_arg(cmd, 1);
    g_return_val_if_fail(target, SRN_ERR);
    g_return_val_if_fail(mode, SRN_ERR);

    return sirc_cmd_mode(srv->irc, target, mode);
}

SrnRet on_command_ctcp(SrnCommand *cmd, void *user_data){
    char timestr[64];
    const char *nick;
    const char *ctcp_cmd;
    const char *msg;

    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    ctcp_cmd = srn_command_get_arg(cmd, 1);
    msg = srn_command_get_arg(cmd, 2);
    g_return_val_if_fail(nick, SRN_ERR);
    g_return_val_if_fail(ctcp_cmd, SRN_ERR);

    LOG_FR("cmd: [%s], msg: [%s]", ctcp_cmd, msg);

    if (strcmp(ctcp_cmd, "PING") == 0 && !msg) { // CTCP ping with out parameter
        unsigned long time;

        time = get_time_since_first_call_ms();
        snprintf(timestr, sizeof(timestr), "%lu", time);
        msg = (const char *)timestr;
    }

    return sirc_cmd_ctcp_req(srv->irc, nick, ctcp_cmd, msg);
}

SrnRet on_command_away(SrnCommand *cmd, void *user_data){
    const char *msg;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    msg = srn_command_get_arg(cmd, 0);

    return sirc_cmd_away(srv->irc, msg);
}

SrnRet on_command_pattern(SrnCommand *cmd, void *user_data){
    const char *subcmd;
    SrnChat *chat;
    SrnRet ret;
    SrnPatternSet *pattern_set;

    subcmd = srn_command_get_subcmd(cmd);
    g_return_val_if_fail(subcmd, SRN_ERR);
    chat = ctx_get_chat(user_data);
    g_return_val_if_fail(chat, SRN_ERR);
    pattern_set = srn_application_get_default()->pattern_set;
    g_return_val_if_fail(pattern_set, SRN_ERR);

    if (g_ascii_strcasecmp(subcmd, "add") == 0){
        const char *name;
        const char *pattern;

        name = srn_command_get_arg(cmd, 0);
        g_return_val_if_fail(name, SRN_ERR);
        pattern = srn_command_get_arg(cmd, 1);
        g_return_val_if_fail(pattern, SRN_ERR);

        ret = srn_pattern_set_add(pattern_set, name, pattern);
        if (!RET_IS_OK(ret)) {
            ret = RET_ERR(_("Failed to add regex pattern \"%1$s\": %2$s"),
                    name, RET_MSG(ret));
        } else {
            ret = RET_OK(_("Regex pattern \"%1$s\" has added"), name);
        }
    } else if (g_ascii_strcasecmp(subcmd, "rm") == 0){
        const char *name;

        name = srn_command_get_arg(cmd, 0);
        g_return_val_if_fail(name, SRN_ERR);

        ret = srn_pattern_set_rm(pattern_set, name);
        if (!RET_IS_OK(ret)) {
            ret = RET_ERR(_("Failed to remove regex pattern \"%1$s\": %2$s"),
                    name, RET_MSG(ret));
        } else {
            ret = RET_OK(_("Regex pattern \"%1$s\" has removed"), name);
        }
    } else if (g_ascii_strcasecmp(subcmd, "list") == 0){
        GList *lst;
        GString *str;

        str = g_string_new(_("Available regex patterns:"));
        lst = srn_pattern_set_list(pattern_set);
        while (lst) {
            const char *name;
            GRegex *regex;

            name = lst->data;
            regex = srn_pattern_set_get(pattern_set, name);
            g_string_append_printf(str, "\n  * %s: %s",
                    name, g_regex_get_pattern(regex));
            lst = g_list_next(lst);
        }
        g_list_free(g_list_first(lst));

        ret = RET_OK("%s", str->str);
        g_string_free(str, TRUE);
    } else {
        g_warn_if_reached();
        ret = SRN_ERR;
    }

    return ret;
}

SrnRet on_command_render(SrnCommand *cmd, void *user_data){
    const char *nick;
    const char *pattern;
    SrnRet ret;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnExtraData *extra_data;
    SrnPatternSet *pattern_set;

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);
    pattern = srn_command_get_arg(cmd, 1);
    g_return_val_if_fail(pattern, SRN_ERR);

    // Check pattern
    pattern_set = srn_application_get_default()->pattern_set;
    if (srn_pattern_set_get(pattern_set, pattern) == NULL) {
        return RET_ERR(_("Pattern \"%1$s\" not found"), pattern);
    }

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    srv_user = srn_server_add_and_get_user(srv, nick);
    g_return_val_if_fail(srv_user, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        SrnChatUser *chat_user;

        chat = ctx_get_chat(user_data);
        g_return_val_if_fail(chat, SRN_ERR);
        chat_user = srn_chat_add_and_get_user(chat, srv_user);
        g_return_val_if_fail(chat_user, SRN_ERR);

        extra_data = chat_user->extra_data;
    } else {
        chat = srv->chat;
        extra_data = srv_user->extra_data;
    }
    g_return_val_if_fail(chat, SRN_ERR);
    g_return_val_if_fail(extra_data, SRN_ERR);

    ret = srn_render_attach_pattern(extra_data, pattern);
    if (!RET_IS_OK(ret)) {
        return RET_ERR(_("Failed to attach pattern to user %1$s\" of chat \"%2$s\": %3$s"),
                srv_user->nick, chat->name, RET_MSG(ret));
    }

    return RET_OK(_("Messages of user \"%1$s\" of chat \"%2$s\" will be rendered by pattern \"%3$s\""),
            srv_user->nick, chat->name, pattern);
}

SrnRet on_command_unrender(SrnCommand *cmd, void *user_data){
    const char *nick;
    const char *pattern;
    SrnRet ret;
    SrnServer *srv;
    SrnChat *chat;
    SrnServerUser *srv_user;
    SrnExtraData *extra_data;

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);
    pattern = srn_command_get_arg(cmd, 1);
    g_return_val_if_fail(pattern, SRN_ERR);

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    srv_user = srn_server_add_and_get_user(srv, nick);
    g_return_val_if_fail(srv_user, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        SrnChatUser *chat_user;

        chat = ctx_get_chat(user_data);
        g_return_val_if_fail(chat, SRN_ERR);
        chat_user = srn_chat_add_and_get_user(chat, srv_user);
        g_return_val_if_fail(chat_user, SRN_ERR);

        extra_data = chat_user->extra_data;
    } else {
        chat = srv->chat;
        extra_data = srv_user->extra_data;
    }
    g_return_val_if_fail(chat, SRN_ERR);
    g_return_val_if_fail(extra_data, SRN_ERR);

    ret = srn_render_detach_pattern(extra_data, pattern);
    if (!RET_IS_OK(ret)) {
        return RET_ERR(_("Failed to dettach pattern from user \"%1$s\": %2$s"),
                srv_user->nick, RET_MSG(ret));
    }

    return RET_OK(_("Messages of user \"%1$s\" of chat \"%2$s\" are no longer rendered by pattern \"%3$s\""),
            srv_user->nick, chat->name, pattern);
}

SrnRet on_command_quote(SrnCommand *cmd, void *user_data){
    SrnServer *srv;
    const char *msg;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    msg = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(msg, SRN_ERR);

    return sirc_cmd_raw(srv->irc, "%s\r\n", msg);
}

SrnRet on_command_clear(SrnCommand *cmd, void *user_data){
    SrnChat *chat;

    chat = ctx_get_chat(user_data);
    g_return_val_if_fail(chat, SRN_ERR);

    sui_buffer_clear_message(chat->ui);

    return SRN_OK;
}

/*******************************************************************************
 * Misc
 ******************************************************************************/

static SrnApplication* ctx_get_app(SrnChatCommandContext *cctx){
    g_return_val_if_fail(cctx, NULL);
    g_return_val_if_fail(cctx->app, NULL);

    return cctx->app;
}

static SrnServer* ctx_get_server(SrnChatCommandContext *cctx){
    g_return_val_if_fail(cctx, NULL);
    g_return_val_if_fail(cctx->srv, NULL);

    return cctx->srv;
}

static SrnChat* ctx_get_chat(SrnChatCommandContext *cctx){
    g_return_val_if_fail(cctx, NULL);
    g_return_val_if_fail(cctx->chat, NULL);

    return cctx->chat;
}
