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
 * @file app_command.c
 * @brief Application command definitions and callbacks
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
#include "filter.h"
#include "decorator.h"
#include "utils.h"

typedef struct _CommandContext {
    SrnApplication *app;
} CommandContext;

static SrnApplication* ctx_get_app(CommandContext *ctx);
static SrnServer* ctx_get_server(CommandContext *ctx);
static SrnChat* ctx_get_chat(CommandContext *ctx);

static SrnRet on_command_reload(SrnCommand *cmd, void *user_data);
static SrnRet on_command_context(SrnCommand *cmd, void *user_data);
static SrnRet on_command_server(SrnCommand *cmd, void *user_data);
static SrnRet on_command_connect(SrnCommand *cmd, void *user_data);
static SrnRet on_command_relay(SrnCommand *cmd, void *user_data);
static SrnRet on_command_unrelay(SrnCommand *cmd, void *user_data);
static SrnRet on_command_ignore(SrnCommand *cmd, void *user_data);
static SrnRet on_command_unignore(SrnCommand *cmd, void *user_data);
static SrnRet on_command_rignore(SrnCommand *cmd, void *user_data);
static SrnRet on_command_unrignore(SrnCommand *cmd, void *user_data);
static SrnRet on_command_query(SrnCommand *cmd, void *user_data);
static SrnRet on_command_unquery(SrnCommand *cmd, void *user_data);
static SrnRet on_command_join(SrnCommand *cmd, void *user_data);
static SrnRet on_command_part(SrnCommand *cmd, void *user_data);
static SrnRet on_command_quit(SrnCommand *cmd, void *user_data);
static SrnRet on_command_topic(SrnCommand *cmd, void *user_data);
static SrnRet on_command_msg(SrnCommand *cmd, void *user_data);
static SrnRet on_command_me(SrnCommand *cmd, void *user_data);
static SrnRet on_command_nick(SrnCommand *cmd, void *user_data);
static SrnRet on_command_whois(SrnCommand *cmd, void *user_data);
static SrnRet on_command_invite(SrnCommand *cmd, void *user_data);
static SrnRet on_command_kick(SrnCommand *cmd, void *user_data);
static SrnRet on_command_mode(SrnCommand *cmd, void *user_data);
static SrnRet on_command_ctcp(SrnCommand *cmd, void *user_data);

SrnCommandBind cmd_binds[] = {
    {
        .name = "/reload",
        .argc = 0,
        .cb = on_command_reload,
    },
    {
        .name = "/context",
        .argc = 2, // <server> [chat]
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_context,
    },
    {
        .name = "/server",
        .subcmd = {"add", "rm", "list", "set", "connect", "disconnect", NULL},
        .argc = 1, // <name>
        .opt = {
            { .key = "-addr",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-tls",            .val = SRN_COMMAND_OPT_NO_VAL },
            { .key = "-tls-noverify",   .val = SRN_COMMAND_OPT_NO_VAL },
            // Backward compatible
            { .key = "-tls-not-verify", .val = SRN_COMMAND_OPT_NO_VAL },
            { .key = "-pwd",            .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-nick",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-user",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-real",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-encode",         .val = SRN_COMMAND_OPT_NO_DEFAULT },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_server,
    },
    {
        .name = "/connect",
        .argc = 2, // <addr> <nick>
        .opt = {
            { .key = "-tls",            .val = SRN_COMMAND_OPT_NO_VAL },
            { .key = "-tls-noverify",   .val = SRN_COMMAND_OPT_NO_VAL },
            // Backward compatible
            { .key = "-tls-not-verify", .val = SRN_COMMAND_OPT_NO_VAL },
            { .key = "-pwd",            .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-user",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-real",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-encode",         .val = SRN_COMMAND_OPT_NO_DEFAULT },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_connect,
    },
    {
        .name = "/relay",
        .argc = 1, // <nick>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_relay,
    },
    {
        .name = "/unrelay",
        .argc = 1, // <nick>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_unrelay,
    },
    {
        .name = "/ignore",
        .argc = 1, // <nick>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_ignore,
    },
    {
        .name = "/unignore",
        .argc = 1, // <nick>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_unignore,
    },
    {
        .name = "/rignore",
        .argc = 2, // <name> <pattern>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_rignore,
    },
    {
        .name = "/unrignore",
        .argc = 1, // <name>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = 0,
        .cb = on_command_unrignore,
    },
    {
        .name = "/query",
        .argc = 1, // <nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_query,
    },
    {
        .name = "/unquery",
        .argc = 1, // <nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_unquery,
    },
    {
        .name = "/join",
        .argc = 1, // <channel>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_join,
    },
    {
        .name = "/part",
        .argc = 2, // <channel> [reason]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_part,
    },
    {
        .name = "/quit",
        .argc = 1, // [reason]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_quit,
    },
    {
        .name = "/topic",
        .argc = 1, // [topic]
        .opt = {
            {.key = "-rm", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_topic,
    },
    {
        .name = "/msg",
        .argc = 2, // <target> <message>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_msg,
    },
    {
        .name = "/me",
        .argc = 1, // <message>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_me,
    },

    {
        .name = "/nick",
        .argc = 1, // <new_nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_nick,
    },
    {
        .name = "/whois",
        .argc = 1, // <nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_whois,
    },
    {
        .name = "/invite",
        .argc = 2, // <nick> <channel>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_invite,
    },
    {
        .name = "/kick",
        .argc = 3, // <nick> <channel> <reason>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_kick,
    },
    {
        .name = "/mode",
        .argc = 2, // <target> <mode>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = 0,
        .cb = on_command_mode,
    },
    {
        .name = "/ctcp",
        .argc = 3, // <nick> <command> <msg>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flag = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_ctcp,
    },
    SRN_COMMAND_EMPTY,
};

static SrnCommandContext cmd_ctx = {
    .binds = cmd_binds,
};

/*******************************************************************************
 * Exported functions
 ******************************************************************************/

/**
 * @brief Run a command in context of SrnApplication
 *
 * @param chat A SrnApplication instance
 * @param cmd
 *
 * @return SRN_OK or SRN_ERR or other error
 */
SrnRet srn_application_run_command(SrnApplication *app, const char *cmd){
    CommandContext ctx;

    g_return_val_if_fail(app, SRN_ERR);
    g_return_val_if_fail(cmd, SRN_ERR);

    ctx.app = app;
    return srn_command_proc(&cmd_ctx, cmd, &ctx);
}

/*******************************************************************************
 * SrnCommand callbacks
 ******************************************************************************/

static SrnRet on_command_reload(SrnCommand *cmd, void *user_data){
    SrnRet ret;
    SrnApplication *app;

    app = ctx_get_app(user_data);

    return srn_application_reload_config(app);
}

static SrnRet on_command_context(SrnCommand *cmd, void *user_data){
    const char *srv_name;
    const char *chat_name;
    SrnApplication *app;
    SrnServer *srv;
    SrnChat *chat;

    app = ctx_get_app(user_data);

    srv_name = srn_command_get_arg(cmd, 0);
    if (!srv_name) {
        return RET_ERR(_("Missing argument <server>"));
    }
    srv = srn_application_get_server(app, srv_name);
    if (!srv) {
        return RET_ERR(_("No such server: %1$s"), srv_name);
    }
    app->cur_srv = srv; // Change current server

    chat_name = srn_command_get_arg(cmd, 1);
    if (!chat_name) {
        // Argument "chat" is optional
        return RET_OK(_("Context switched to server \"%1$s\""), srv_name);
    }
    chat = srn_server_get_chat(srv, chat_name);
    if (!chat) {
        return RET_ERR(_("No such chat: %1$s") , chat_name);
    }
    srv->cur_chat = chat;

    return RET_OK(_("Context switched to server \"%1$s\", chat \"%2$s\""),
            srv_name, chat_name);
}

static SrnRet on_command_server(SrnCommand *cmd, void *user_data){
    const char *subcmd;
    const char *name;
    const char *addr;
    const char *passwd;
    const char *nick;
    const char *user;
    const char *real;
    const char *encoding;
    SrnRet ret;
    SrnApplication *app;
    SrnServer *srv;
    SrnServerConfig *cfg;

    app = ctx_get_app(user_data);
    subcmd = srn_command_get_subcmd(cmd);
    g_return_val_if_fail(subcmd, SRN_ERR);

    if (g_ascii_strcasecmp(subcmd, "list") == 0){
        char *dump;

        dump = srn_application_dump_server_config_list(app);
        ret = RET_OK("%s", dump);
        g_free(dump);

        return ret;
    }

    name = srn_command_get_arg(cmd, 0);
    // Get server config
    if (g_ascii_strcasecmp(subcmd, "add") == 0){
        ret = srn_application_add_server_config(app, name);
        if (!RET_IS_OK(ret)){
            return RET_ERR(_("Failed to create server config: %1$s"),
                    RET_MSG(ret));
        }
    }

    cfg = srn_application_get_server_config(app, name);
    if (!cfg){
        return RET_ERR(_("No such server: %1$s"), name);
    }

    if (g_ascii_strcasecmp(subcmd, "add") == 0
            || g_ascii_strcasecmp(subcmd, "set") == 0){
        if (srn_command_get_opt(cmd, "-pwd", &passwd)){
            str_assign(&cfg->passwd, passwd);
        }
        if (srn_command_get_opt(cmd, "-nick", &nick)){
            str_assign(&cfg->nickname, nick);
        }
        if (srn_command_get_opt(cmd, "-user", &user)){
            str_assign(&cfg->username, user);
        }
        if (srn_command_get_opt(cmd, "-real", &real)){
            str_assign(&cfg->realname, real);
        }
        if (srn_command_get_opt(cmd, "-encode", &encoding)){
            str_assign(&cfg->irc->encoding, encoding);
        }
        if (srn_command_get_opt(cmd, "-tls", NULL)){
            cfg->irc->tls = TRUE;
        }
        if (srn_command_get_opt(cmd, "-tls-noverify", NULL)){
            cfg->irc->tls = TRUE;
            cfg->irc->tls_noverify = TRUE;
        } else if (srn_command_get_opt(cmd, "-tls-not-verify", NULL)){
            // Backward compatible
            cfg->irc->tls = TRUE;
            cfg->irc->tls_noverify = TRUE;
        }
        if (srn_command_get_opt(cmd, "-addr", &addr)){
            ret = srn_server_config_set_addr(cfg, addr);
            if (!RET_IS_OK(ret)){
                return RET_ERR(_("Failed to resolve server address: %s"),
                        RET_MSG(ret));
            }
        }

        if (g_ascii_strcasecmp(subcmd, "add") == 0){
            return RET_OK(_("Server config \"%1$s\" is created"), name);
        } else {
            return RET_OK(_("Server config \"%1$s\" is motified"), name);
        }
    }

    if (g_ascii_strcasecmp(subcmd, "connect") == 0){
        srv = srn_application_get_server(app, name);
        if (!srv) { // Create one
            ret = srn_server_config_check(cfg);
            if (!RET_IS_OK(ret)){
                return ret;
            }
            ret = srn_application_add_server(app, cfg);
            if (!RET_IS_OK(ret)){
                return RET_ERR(_("Failed to instantiate server \"%1$s\""), name);
            }
            srv = srn_application_get_server(app, name);
        }

        ret = srn_server_connect(srv);
        if (!RET_IS_OK(ret)) {
            return ret;
        }

        srn_server_wait_until_registered(srv); // FIXME: busy wait
        if (!srn_server_is_registered(srv)){
            return RET_ERR(_("Failed to register on server \"%1$s\""), cfg->name);
        }

        return SRN_OK;
    }

    if (g_ascii_strcasecmp(subcmd, "disconnect") == 0){
        SrnServerState prev_state;

        srv = srn_application_get_server(app, name);
        if (!srv) {
            // FIXME: better errmsg?
            return RET_ERR(_("Can not disconnect from a unconnected server"));
        }

        // FIXME: looks tricky
        prev_state = srv->state;
        ret = srn_server_disconnect(srv);
        if (RET_IS_OK(ret) && prev_state == SRN_SERVER_STATE_RECONNECTING){
            ret = RET_OK(_("Reconnection stopped"));
        }

        return ret;
    }

    if (g_ascii_strcasecmp(subcmd, "rm") == 0){
        srv = srn_application_get_server(app, name);
        if (srv) {
            return srn_server_state_transfrom(srv, SRN_SERVER_ACTION_QUIT);
        }

        cfg = srn_application_get_server_config(app, name);
        if (!cfg){
            return RET_ERR(_("No such server: %1$s"), name);
        } else if (cfg->predefined){
            return RET_ERR(_("Can not remove a predefined server: %1$s"), name);
        } else {
            srn_server_config_free(cfg);
        }

        return RET_OK(_("Server \"%1$s\" is removed"), name);
    }

    return RET_ERR(_("Unknown sub command: %1$s"), subcmd);
}

static SrnRet on_command_connect(SrnCommand *cmd, void *user_data){
    const char *addr;
    const char *passwd;
    const char *nick;
    const char *user;
    const char *real;
    const char *encoding;
    SrnApplication *app;
    SrnServer *srv = NULL;
    SrnServerConfig *cfg = NULL;
    SrnRet ret = SRN_OK;

    app = ctx_get_app(user_data);
    addr = srn_command_get_arg(cmd, 0);
    nick = srn_command_get_arg(cmd, 1);

    ret = srn_application_add_server_config(app, addr);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to create server config: %s"), RET_MSG(ret));
        goto FIN;
    }
    cfg = srn_application_get_server_config(app, addr);

    str_assign(&cfg->nickname, nick);
    if (srn_command_get_opt(cmd, "-pwd", &passwd)){
        str_assign(&cfg->passwd, passwd);
    }
    if (srn_command_get_opt(cmd, "-user", &user)){
        str_assign(&cfg->username, user);
    }
    if (srn_command_get_opt(cmd, "-real", &real)){
        str_assign(&cfg->realname, real);
    }
    if (srn_command_get_opt(cmd, "-encode", &encoding)){
        str_assign(&cfg->irc->encoding, encoding);
    }
    cfg->irc->tls = srn_command_get_opt(cmd, "-tls", NULL);
    cfg->irc->tls_noverify = srn_command_get_opt(cmd, "-tls-noverify", NULL);
    if (cfg->irc->tls_noverify){
        cfg->irc->tls = TRUE;
    }
    // Backward compatible
    if (!cfg->irc->tls_noverify){
        cfg->irc->tls_noverify = srn_command_get_opt(cmd, "-tls-not-verify", NULL);
        if (cfg->irc->tls_noverify){
            cfg->irc->tls = TRUE;
        }
    }

    ret = srn_server_config_set_addr(cfg, addr);
    if (!RET_IS_OK(ret)){
        ret = RET_ERR(_("Failed to resolve server address: %s"), RET_MSG(ret));
        goto FIN;
    }

    ret = srn_server_config_check(cfg);
    if (!RET_IS_OK(ret)) {
        goto FIN;
    }
    ret = srn_application_add_server(app, cfg);
    if (!RET_IS_OK(ret)) {
        ret = RET_ERR(_("Failed to instantiate server \"%1$s\""), cfg->name);
        goto FIN;
    }
    srv = srn_application_get_server(app, cfg->name);

    ret = srn_server_connect(srv);
    if (!RET_IS_OK(ret)){
        goto FIN;
    }

    srn_server_wait_until_registered(srv);
    if (!srn_server_is_registered(srv)){
        ret = RET_ERR(_("Failed to register on server \"%1$s\""), cfg->name);
        goto FIN;
    }

    return SRN_OK;
FIN:
    if (srv){
        srn_application_rm_server(app, srv);
    }
    if (cfg){
        srn_application_rm_server_config(app, cfg);
    }

    return ret;
}

static SrnRet on_command_relay(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

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

    return relay_decroator_add_nick(chat, nick);
}

static SrnRet on_command_unrelay(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

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

    return relay_decroator_rm_nick(chat, nick);
}

static SrnRet on_command_ignore(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

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

    return nick_filter_add_nick(chat, nick);
}

static SrnRet on_command_unignore(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    return nick_filter_rm_nick(chat, nick);
}

static SrnRet on_command_rignore(SrnCommand *cmd, void *user_data){
    const char *name;
    const char *pattern;
    SrnServer *srv;
    SrnChat *chat;

    name = srn_command_get_arg(cmd, 0);
    pattern = srn_command_get_arg(cmd, 1);
    g_return_val_if_fail(name, SRN_ERR);
    g_return_val_if_fail(pattern, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    g_return_val_if_fail(chat, SRN_ERR);

    return regex_filter_add_pattern(chat, name, pattern);
}

static SrnRet on_command_unrignore(SrnCommand *cmd, void *user_data){
    const char *name;
    SrnServer *srv;
    SrnChat *chat;

    name = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(name, SRN_ERR);

    if (srn_command_get_opt(cmd, "-cur", NULL)){
        chat = ctx_get_chat(user_data);
    } else {
        srv = ctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    return regex_filter_rm_pattern(chat, name);
}

static SrnRet on_command_query(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return srn_server_add_chat(srv, nick);
}

static SrnRet on_command_unquery(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;
    SrnChat *chat;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    chat = srn_server_get_chat(srv, nick);
    if (!chat){
        chat = srv->cur_chat;
    }

    return srn_server_rm_chat(srv, chat);
}

static SrnRet on_command_join(SrnCommand *cmd, void *user_data){
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

static SrnRet on_command_part(SrnCommand *cmd, void *user_data){
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

static SrnRet on_command_quit(SrnCommand *cmd, void *user_data){
    const char *reason;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    reason = srn_command_get_arg(cmd, 0);
    if (!reason) reason = PACKAGE_NAME " " PACKAGE_VERSION " " "quit.";

    return sirc_cmd_quit(srv->irc, reason);
}

static SrnRet on_command_topic(SrnCommand *cmd, void *user_data){
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

static SrnRet on_command_msg(SrnCommand *cmd, void *user_data){
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

static SrnRet on_command_me(SrnCommand *cmd, void *user_data){
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
        return RET_ERR(_("Can not send message to a server"));
    }

    ret = sirc_cmd_action(chat->srv->irc, chat->name, msg);
    if (!RET_IS_OK(ret)){
        return RET_ERR(_("Failed to send action message: %1$s"), RET_MSG(ret));
    }

    srn_chat_add_action_message(chat, chat->user, msg);

    return SRN_OK;
}

static SrnRet on_command_nick(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_nick(srv->irc, nick);
}

static SrnRet on_command_whois(SrnCommand *cmd, void *user_data){
    const char *nick;
    SrnServer *srv;

    srv = ctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = srn_command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_whois(srv->irc, nick);
}

static SrnRet on_command_invite(SrnCommand *cmd, void *user_data){
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

    return sirc_cmd_invite(srv->irc, chan, nick);
}

static SrnRet on_command_kick(SrnCommand *cmd, void *user_data){
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

static SrnRet on_command_mode(SrnCommand *cmd, void *user_data){
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

static SrnRet on_command_ctcp(SrnCommand *cmd, void *user_data){
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

/*******************************************************************************
 * Misc
 ******************************************************************************/

static SrnApplication* ctx_get_app(CommandContext *ctx){
    g_return_val_if_fail(ctx, NULL);
    g_return_val_if_fail(ctx->app, NULL);
    return ctx->app;
}

static SrnServer* ctx_get_server(CommandContext *ctx){
    SrnApplication *app;

    app = ctx_get_app(ctx);
    g_return_val_if_fail(app, NULL);

    return app->cur_srv;
}

static SrnChat* ctx_get_chat(CommandContext *ctx){
    SrnServer *srv;

    srv = ctx_get_server(ctx);
    g_return_val_if_fail(srv, NULL);

    return srv->cur_chat;
}
