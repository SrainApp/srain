/**
 * @file server_cmd.c
 * @brief Server comand callback
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-26
 */

#define __DBG_ON
#define __LOG_ON

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <glib.h>

#include "server.h"
#include "server_cmd.h"

#include "sui/sui.h"

#include "srain.h"
#include "log.h"
#include "meta.h"
#include "i18n.h"
#include "command.h"
#include "filter.h"
#include "decorator.h"

typedef struct _ServerCmdContext {
    Chat *chat;
} ServerCmdContext;

static Server* scctx_get_server(ServerCmdContext *scctx);
static Chat* scctx_get_chat(ServerCmdContext *scctx);

static int on_command_connect(Command *cmd, void *user_data);
static int on_command_relay(Command *cmd, void *user_data);
static int on_command_unrelay(Command *cmd, void *user_data);
static int on_command_ignore(Command *cmd, void *user_data);
static int on_command_unignore(Command *cmd, void *user_data);
static int on_command_rignore(Command *cmd, void *user_data);
static int on_command_unrignore(Command *cmd, void *user_data);
static int on_command_query(Command *cmd, void *user_data);
static int on_command_unquery(Command *cmd, void *user_data);
static int on_command_join(Command *cmd, void *user_data);
static int on_command_part(Command *cmd, void *user_data);
static int on_command_quit(Command *cmd, void *user_data);
static int on_command_topic(Command *cmd, void *user_data);
static int on_command_msg(Command *cmd, void *user_data);
static int on_command_me(Command *cmd, void *user_data);
static int on_command_nick(Command *cmd, void *user_data);
static int on_command_whois(Command *cmd, void *user_data);
static int on_command_invite(Command *cmd, void *user_data);
static int on_command_kick(Command *cmd, void *user_data);
static int on_command_mode(Command *cmd, void *user_data);
static int on_command_list(Command *cmd, void *user_data);

static void on_unknown_cmd(const char *cmd, void *user_data);
static void on_unknown_opt(Command *cmd, const char *opt, void *user_data);
static void on_missing_opt_val(Command *cmd, const char *opt, void *user_data);
static void on_too_many_opt(Command *cmd, void *user_data);
static void on_too_many_arg(Command *cmd, void *user_data);
static void on_callback_fail(Command *cmd, void *user_data);

static CommandBind cmd_binds[] = {
    {
        "/connect", 2, // <hosts> <nick>
        {"-port", "-ssl", "-passwd", "-realname", NULL},
        {"6667", "off", "", "Can you can a can?", NULL},
        on_command_connect
    },
    {
        "/relay", 1, // <nick>
        {"-cur", NULL},
        {NULL},
        on_command_relay
    },
    {
        "/unrelay", 1, // <nick>
        {"-cur", NULL},
        {NULL},
        on_command_unrelay
    },
    {
        "/ignore", 1, // <nick>
        {"-cur", NULL},
        {NULL, NULL},
        on_command_ignore
    },
    {
        "/unignore", 1, // <nick>
        {"-cur", NULL},
        {NULL},
        on_command_unignore
    },
    {
        "/rignore", 2, // <name> <pattern>
        {"-cur", NULL},
        {NULL, NULL},
        on_command_rignore
    },
    {
        "/unrignore", 2, // <name> <pattern>
        {"-cur", NULL},
        {NULL},
        on_command_unrignore
    },
    {
        "/query", 1, // <nick>
        {NULL},
        {NULL},
        on_command_query
    },
    {
        "/unquery", 1, // <nick>
        {NULL},
        {NULL},
        on_command_unquery
    },
    {
        "/join", 1, // <channel>
        {NULL},
        {NULL},
        on_command_join
    },
    {
        "/part", 2, // <channel> <reason>
        {NULL},
        {NULL},
        on_command_part
    },
    {
        "/quit", 1, // <reason>
        {NULL},
        {NULL},
        on_command_quit
    },
    {
        "/topic", 1, // <topic>
        {"-rm", NULL},
        {NULL, NULL},
        on_command_topic
    },
    {
        "/msg", 2, // <target> <message>
        {NULL},
        {NULL},
        on_command_msg
    },
    {
        "/me", 1, // <message>
        {NULL},
        {NULL},
        on_command_me
    },

    {
        "/nick", 1, // <new_nick>
        {NULL},
        {NULL},
        on_command_nick
    },
    {
        "/whois", 1, // <nick>
        {NULL},
        {NULL},
        on_command_whois
    },
    {
        "/invite", 2, // <nick> <channel>
        {NULL},
        {NULL},
        on_command_invite
    },
    {
        "/kick", 3, // <nick> <channel> <reason>
        {NULL},
        {NULL},
        on_command_kick
    },
    {
        "/mode", 2, // <target> <mode>
        {NULL},
        {NULL},
        on_command_mode
    },
    {
        "/list", 1, // <channel>
        {NULL},
        {NULL},
        on_command_list
    },
    {
        NULL, 0, {NULL}, {NULL}, NULL
    },
};

static CommandContext cmd_ctx = {
    .binds = cmd_binds,
    .on_unknown_cmd = on_unknown_cmd,
    .on_unknown_opt = on_unknown_opt,
    .on_missing_opt_val = on_missing_opt_val,
    .on_too_many_opt = on_too_many_opt,
    .on_too_many_arg = on_too_many_arg,
    .on_callback_fail = on_callback_fail,
};

static Server *def_srv; // Default server

/*******************************************************************************
 * Exported functions
 ******************************************************************************/

void server_cmd_init(){
    commmad_set_context(&cmd_ctx);
}

/**
 * @brief server_cmd Run a command in specified Chat
 *
 * @param chat Can be NULL
 * @param cmd
 *
 * @return SRN_ERR or result of `command_proc()`
 */
int server_cmd(Chat *chat, const char *cmd){
    ServerCmdContext scctx;

    g_return_val_if_fail(cmd, SRN_ERR);

    scctx.chat = chat;

    return command_proc(cmd, &scctx);
}

/*******************************************************************************
 * Command callbacks
 ******************************************************************************/

static int on_command_connect(Command *cmd, void *user_data){
    const char *host;
    const char *nick;
    char *port, *passwd, *ssl, *realname;
    bool use_ssl;

    host = command_get_arg(cmd, 0);
    nick = command_get_arg(cmd, 1);

    g_return_val_if_fail(host, SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);

    command_get_opt(cmd, "-port", &port);
    command_get_opt(cmd, "-passwd", &passwd);
    command_get_opt(cmd, "-ssl", &ssl);
    command_get_opt(cmd, "-realname", &realname);

    use_ssl = (strcmp(ssl, "on") == 0);

    Server *srv = server_new(host, host, atoi(port), passwd, use_ssl, "UTF-8",
            nick, PACKAGE_NAME, realname);

    g_return_val_if_fail(srv, SRN_ERR);

    def_srv = srv;
    server_connect(def_srv);
    while (def_srv->stat == SERVER_CONNECTING) sui_proc_pending_event();

    if (def_srv->stat != SERVER_CONNECTED) {
        def_srv = NULL;
        return SRN_ERR;
    }

    return SRN_OK;
}

static int on_command_relay(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;
    Chat *chat;

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    g_return_val_if_fail(chat, SRN_ERR);

    return relay_decroator_add_nick(chat, nick);
}

static int on_command_unrelay(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;
    Chat *chat;

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    g_return_val_if_fail(chat, SRN_ERR);

    return relay_decroator_rm_nick(chat, nick);
}

static int on_command_ignore(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;
    Chat *chat;

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    g_return_val_if_fail(chat, SRN_ERR);

    return nick_filter_add_nick(chat, nick);
}

static int on_command_unignore(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;
    Chat *chat;

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    return nick_filter_rm_nick(chat, nick);
}

static int on_command_rignore(Command *cmd, void *user_data){
    const char *name;
    const char *pattern;
    Server *srv;
    Chat *chat;

    name = command_get_arg(cmd, 0);
    pattern = command_get_arg(cmd, 1);
    g_return_val_if_fail(name, SRN_ERR);
    g_return_val_if_fail(pattern, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    g_return_val_if_fail(chat, SRN_ERR);

    return regex_filter_add_pattern(chat, name, pattern);
}

static int on_command_unrignore(Command *cmd, void *user_data){
    const char *name;
    const char *pattern;
    Server *srv;
    Chat *chat;

    name = command_get_arg(cmd, 0);
    pattern = command_get_arg(cmd, 1);
    g_return_val_if_fail(name, SRN_ERR);
    g_return_val_if_fail(pattern, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    return regex_filter_rm_pattern(chat, pattern);
}

static int on_command_query(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return server_add_chat(srv, nick);
}

static int on_command_unquery(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return server_rm_chat(srv, nick);
}

static int on_command_join(Command *cmd, void *user_data){
    const char *chan;
    const char *passwd;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    chan = command_get_arg(cmd, 0);
    passwd = command_get_arg(cmd, 1);

    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_join(srv->irc, chan, passwd);
}

static int on_command_part(Command *cmd, void *user_data){
    const char *chan;
    const char *reason;
    Server *srv;
    Chat *chat;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);
    chat = scctx_get_chat(user_data);

    LOG_FR("chat: %p", chat);

    chan = command_get_arg(cmd, 0);
    reason = command_get_arg(cmd, 1);

    if (!chan && chat){
        chan = chat->name;
    }
    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_part(srv->irc, chan, reason);
}

static int on_command_quit(Command *cmd, void *user_data){
    const char *reason;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    reason = command_get_arg(cmd, 0);
    if (!reason) reason = PACKAGE_NAME " " PACKAGE_VERSION " " "quit.";

    return sirc_cmd_quit(srv->irc, reason);
}

static int on_command_topic(Command *cmd, void *user_data){
    const char *topic;
    Server *srv;
    Chat *chat;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);
    g_return_val_if_fail(chat = scctx_get_chat(user_data), SRN_ERR);

    topic = command_get_arg(cmd, 0);

    if (command_get_opt(cmd, "-rm", NULL)) topic = "";

    return sirc_cmd_topic(srv->irc, chat->name, topic);
}

static int on_command_msg(Command *cmd, void *user_data){
    const char *target;
    const char *msg;
    Server *srv;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    msg = command_get_arg(cmd, 1);
    target = command_get_arg(cmd, 0);
    g_return_val_if_fail(msg, SRN_ERR);
    g_return_val_if_fail(target, SRN_ERR);

    /* Note: we can not use chat_add_sent_message() here, for there is maybe no
     * a Chat named `target`.
     * TODO: A better way?
     */
    if (sirc_cmd_msg(srv->irc, target, msg) == SRN_OK){
        chat_add_misc_message_fmt(srv->cur_chat, srv->user->nick,
                "A message has been sent to \"%s\"", target);
        return SRN_OK;
    } else {
        return SRN_ERR;
    }
}

static int on_command_me(Command *cmd, void *user_data){
    const char *msg;
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_val_if_fail(chat, SRN_ERR);

    msg = command_get_arg(cmd, 0);
    g_return_val_if_fail(msg, SRN_ERR);

    chat_add_action_message(chat, chat->user->nick, msg);

    return SRN_OK;
}

static int on_command_nick(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_nick(srv->irc, nick);
}

static int on_command_whois(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_whois(srv->irc, nick);
}

static int on_command_invite(Command *cmd, void *user_data){
    const char *nick;
    const char *chan;
    Server *srv;
    Chat *chat;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);
    chat = scctx_get_chat(user_data);

    nick = command_get_arg(cmd, 0);
    chan = command_get_arg(cmd, 1);
    g_return_val_if_fail(nick, SRN_ERR);

    if (!chan && chat) {
        chan = chat->name;
    }
    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_invite(srv->irc, chan, nick);
}

static int on_command_kick(Command *cmd, void *user_data){
    const char *nick;
    const char *chan;
    const char *reason;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    nick = command_get_arg(cmd, 0);
    chan = command_get_arg(cmd, 1);
    reason = command_get_arg(cmd, 3);
    g_return_val_if_fail(nick, SRN_ERR);
    g_return_val_if_fail(chan, SRN_ERR);

    return sirc_cmd_kick(srv->irc, nick, chan, reason);
}

static int on_command_mode(Command *cmd, void *user_data){
    const char *mode;
    const char *target;
    Server *srv;

    g_return_val_if_fail(srv = scctx_get_server(user_data), SRN_ERR);

    mode = command_get_arg(cmd, 0);
    target = command_get_arg(cmd, 1);
    g_return_val_if_fail(mode, SRN_ERR);
    g_return_val_if_fail(target, SRN_ERR);

    return sirc_cmd_mode(srv->irc, target, mode);
}

static int on_command_list(Command *cmd, void *user_data){
    return SRN_OK;
}

/*******************************************************************************
 * Error callbacks
 ******************************************************************************/

static void on_unknown_cmd(const char *cmd, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick,
            _("No such command: %s"), cmd);
}

static void on_unknown_opt(Command *cmd, const char *opt, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick,
            _("No such option: %s"), opt);
}

static void on_missing_opt_val(Command *cmd, const char *opt, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick,
            _("Option missing value: %s"), opt);
}

static void on_too_many_opt(Command *cmd, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick, _("Too many options"));
}

static void on_too_many_arg(Command *cmd, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick, _("Too many arguments"));
}

static void on_callback_fail(Command *cmd, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick,
            _("Command failed: %s"), cmd->rawcmd);
}

/*******************************************************************************
 * Misc
 ******************************************************************************/

/* Get Server from ServerCmdContext, If Server is NULL, fallback to def_srv */
static Server* scctx_get_server(ServerCmdContext *scctx){
    g_return_val_if_fail(scctx, NULL);

    if (scctx->chat){
        return scctx->chat->srv;
    } else {
        return def_srv;
    }
}

static Chat* scctx_get_chat(ServerCmdContext *scctx){
    g_return_val_if_fail(scctx, NULL);

    if (scctx->chat){
        return scctx->chat;
    } else {
        return def_srv ? def_srv->chat : NULL;
    }
}
