/**
 * @file server_cmd.c
 * @brief Server comand callback
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-07-26
 */


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
#include "prefs.h"
#include "utils.h"

typedef struct _ServerCmdContext {
    Chat *chat;
} ServerCmdContext;

static Server* scctx_get_server(ServerCmdContext *scctx);
static Chat* scctx_get_chat(ServerCmdContext *scctx);

static int on_command_server(Command *cmd, void *user_data);
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
static void on_missing_arg(Command *cmd, int narg, void *user_data);
static void on_missing_opt_val(Command *cmd, const char *opt, void *user_data);
static void on_too_many_opt(Command *cmd, void *user_data);
static void on_too_many_arg(Command *cmd, void *user_data);
static void on_callback_fail(Command *cmd, void *user_data);

CommandBind cmd_binds[] = {
    {
        .name = "/server",
        .subcmd = {"add", "rm", "set", "connect", "disconnect", NULL},
        .argc = 1, // <name>
        .opt_key =
        {"-host", "-port", "-tls", "-tls-valid-all", "-pwd", "-nick", "-user", "-real", "-encode",  NULL},
        .opt_default_val =
        {NULL, "6667", NULL, NULL, "Zaidan", "Srain", "Can you can a can?", "UTF-8", NULL},
        .flag = 0,
        .cb = on_command_server,
    },
    {
        .name = "/connect",
        .argc = 2, // <hosts> <nick>
        .opt_key =
        {"-host", "-port", "-tls", "-tls-valid-all", "-pwd", "-nick", "-user", "-real", "-encode",  NULL},
        .opt_default_val =
        {NULL, "6667", NULL, NULL, "Zaidan", "Srain", "Can you can a can?", "UTF-8", NULL},
        .flag = 0,
        .cb = on_command_connect,
    },
    {
        .name = "/relay",
        .argc = 1, // <nick>
        .opt_key = {"-cur", NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_relay,
    },
    {
        .name = "/unrelay",
        .argc = 1, // <nick>
        .opt_key = {"-cur", NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_unrelay,
    },
    {
        .name = "/ignore",
        .argc = 1, // <nick>
        .opt_key = {"-cur", NULL},
        .opt_default_val = {NULL, NULL},
        .flag = 0,
        .cb = on_command_ignore,
    },
    {
        .name = "/unignore",
        .argc = 1, // <nick>
        .opt_key = {"-cur", NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_unignore,
    },
    {
        .name = "/rignore",
        .argc = 2, // <name> <pattern>
        .opt_key = {"-cur", NULL},
        .opt_default_val = {NULL, NULL},
        .flag = 0,
        .cb = on_command_rignore,
    },
    {
        .name = "/unrignore",
        .argc = 1, // <name>
        .opt_key = {"-cur", NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_unrignore,
    },
    {
        .name = "/query",
        .argc = 1, // <nick>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_query,
    },
    {
        .name = "/unquery",
        .argc = 1, // <nick>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_unquery,
    },
    {
        .name = "/join",
        .argc = 1, // <channel>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_join,
    },
    {
        .name = "/part",
        .argc = 2, // <channel> <reason>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_part,
    },
    {
        .name = "/quit",
        .argc = 1, // <reason>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_quit,
    },
    {
        .name = "/topic",
        .argc = 1, // <topic>
        .opt_key = {"-rm", NULL},
        .opt_default_val = {NULL, NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_topic,
    },
    {
        .name = "/msg",
        .argc = 2, // <target> <message>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_msg,
    },
    {
        .name = "/me",
        .argc = 1, // <message>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_me,
    },

    {
        .name = "/nick",
        .argc = 1, // <new_nick>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_nick,
    },
    {
        .name = "/whois",
        .argc = 1, // <nick>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_whois,
    },
    {
        .name = "/invite",
        .argc = 2, // <nick> <channel>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_invite,
    },
    {
        .name = "/kick",
        .argc = 3, // <nick> <channel> <reason>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_kick,
    },
    {
        .name = "/mode",
        .argc = 2, // <target> <mode>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_mode,
    },
    {
        .name = "/list",
        .argc = 1, // <channel>
        .opt_key = {NULL},
        .opt_default_val = {NULL},
        .flag = 0,
        .cb = on_command_list,
    },
    COMMAND_EMPTY,
};

static CommandContext cmd_ctx = {
    .binds = cmd_binds,
    .on_unknown_cmd = on_unknown_cmd,
    .on_unknown_opt = on_unknown_opt,
    .on_missing_opt_val = on_missing_opt_val,
    .on_missing_arg = on_missing_arg,
    .on_too_many_opt = on_too_many_opt,
    .on_too_many_arg = on_too_many_arg,
    .on_callback_fail = on_callback_fail,
};

static Server *def_srv; // Default server

/*******************************************************************************
 * Exported functions
 ******************************************************************************/

void server_cmd_init(){
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

    return command_proc(&cmd_ctx, cmd, &scctx);
}

/*******************************************************************************
 * Command callbacks
 ******************************************************************************/

static int on_command_server(Command *cmd, void *user_data){
    char *errmsg;
    const char *subcmd;
    const char *name;
    const char *host;
    const char *port;
    const char *passwd;
    const char *nick;
    const char *user;
    const char *real;
    const char *encoding;

    Server *srv;
    ServerPrefs *prefs;

    name = command_get_arg(cmd, 0);
    g_return_val_if_fail(name, SRN_ERR);

    subcmd = cmd->subcmd;
    if (!subcmd) {
        ERR_FR("No subcommand specified");
        return SRN_ERR;
    }

    if (g_ascii_strcasecmp(subcmd, "add") == 0){
        prefs = server_prefs_new(name);
    } else {
        prefs = server_prefs_get_prefs(name);
    }

    if (!prefs){
        ERR_FR("Failed to get ServerPrefs '%s'", host);
    }

    if (g_ascii_strcasecmp(subcmd, "add") == 0){
        errmsg = prefs_read_server_prefs(prefs);
        if (errmsg){
            ERR_FR("%s", errmsg);
            g_free(errmsg);
            return SRN_ERR;
        }
    }

    if (g_ascii_strcasecmp(subcmd, "add") == 0
            || g_ascii_strcasecmp(subcmd, "set") == 0){
        if (command_get_opt(cmd, "-host", &host)){
            str_assign(&prefs->host, host);
        }
        if (command_get_opt(cmd, "-port", &port)){
            prefs->port = atoi(port);
        }
        if (command_get_opt(cmd, "-pwd", &passwd)){
            str_assign(&prefs->passwd, passwd);
        }
        if (command_get_opt(cmd, "-nick", &nick)){
            str_assign(&prefs->nickname, nick);
        }
        if (command_get_opt(cmd, "-user", &user)){
            str_assign(&prefs->username, user);
        }
        if (command_get_opt(cmd, "-real", &real)){
            str_assign(&prefs->realname, real);
        }
        if (command_get_opt(cmd, "-encode", &encoding)){
            str_assign(&prefs->encoding, encoding);
        }
        if (command_get_opt(cmd, "-tls", NULL)){
            prefs->irc->use_ssl = true;
        }
        if (command_get_opt(cmd, "-tls-vaild-all", NULL)){
            prefs->irc->verify_ssl_cert = false;
        }

        return SRN_OK;
    }

    if (g_ascii_strcasecmp(subcmd, "connect") == 0){
        srv = server_list_get_server(name);
        if (!srv) { // Create one
            if (!server_prefs_is_valid(prefs)){
                ERR_FR("Not completed ServerPrefs");
                return SRN_ERR;
            }
            srv = server_new_from_prefs(prefs);
            g_return_val_if_fail(srv, SRN_ERR);
        }

        def_srv = srv;
        server_connect(srv);
        server_wait_until_registered(def_srv);

        if (!server_is_registered(srv)){
            def_srv = NULL;
            return SRN_ERR;
        }

        return SRN_OK;
    }

    if (g_ascii_strcasecmp(subcmd, "disconnect") == 0){
        srv = server_list_get_server(name);
        g_return_val_if_fail(srv, SRN_ERR);

        if (srv->stat != SERVER_CONNECTED){
            return SRN_ERR;
        }

        server_disconnect(srv);
        return SRN_OK;
    }

    if (g_ascii_strcasecmp(subcmd, "rm") == 0){
        srv = server_list_get_server(name);
        if (srv) {
            if (srv->stat != SERVER_DISCONNECTED) {
                return SRN_ERR;
            }
            server_free(srv);
        }

       prefs = server_prefs_get_prefs(name);
       if (prefs){
           server_prefs_free(prefs);
       }
    }

    ERR_FR("Unknown subcommand: %s", subcmd);

    return SRN_ERR;
}

static int on_command_connect(Command *cmd, void *user_data){
    char *errmsg;
    const char *host;
    const char *port;
    const char *passwd;
    const char *nick;
    const char *user;
    const char *real;
    const char *encoding;
    Server *srv;
    ServerPrefs *prefs;

    host = command_get_arg(cmd, 0);
    nick = command_get_arg(cmd, 1);

    g_return_val_if_fail(host, SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);

    prefs = server_prefs_new(host);
    if (!prefs){
        ERR_FR("Failed to create ServerPrefs '%s'", host);
    }

    errmsg = prefs_read_server_prefs(prefs);
    if (errmsg){
        // TODO ...
        ERR_FR("%s", errmsg);
        server_prefs_free(prefs);
        g_free(errmsg);

        return SRN_ERR;
    }

    if (!server_prefs_is_valid(prefs)){
        // TODO
        ERR_FR("Not completed ServerPrefs");
        return SRN_ERR;
    }

    if (command_get_opt(cmd, "-host", &host)){
        str_assign(&prefs->host, host);
    }
    if (command_get_opt(cmd, "-port", &port)){
        prefs->port = atoi(port);
    }
    if (command_get_opt(cmd, "-pwd", &passwd)){
        str_assign(&prefs->passwd, passwd);
    }
    if (command_get_opt(cmd, "-nick", &nick)){
        str_assign(&prefs->nickname, nick);
    }
    if (command_get_opt(cmd, "-user", &user)){
        str_assign(&prefs->username, user);
    }
    if (command_get_opt(cmd, "-real", &real)){
        str_assign(&prefs->realname, real);
    }
    if (command_get_opt(cmd, "-encode", &encoding)){
        str_assign(&prefs->encoding, encoding);
    }
    if (command_get_opt(cmd, "-tls", NULL)){
        prefs->irc->use_ssl = true;
    }
    if (command_get_opt(cmd, "-tls-vaild-all", NULL)){
        prefs->irc->verify_ssl_cert = false;
    }

    srv = server_new_from_prefs(prefs);
    g_return_val_if_fail(srv, SRN_ERR);

    def_srv = srv;
    server_connect(def_srv);

    server_wait_until_registered(def_srv);

    if (!server_is_registered(srv)){
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
    Server *srv;
    Chat *chat;

    name = command_get_arg(cmd, 0);
    g_return_val_if_fail(name, SRN_ERR);

    if (command_get_opt(cmd, "-cur", NULL)){
        chat = scctx_get_chat(user_data);
    } else {
        srv = scctx_get_server(user_data);
        g_return_val_if_fail(srv, SRN_ERR);
        chat = srv->chat;
    }

    return regex_filter_rm_pattern(chat, name);
}

static int on_command_query(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return server_add_chat(srv, nick);
}

static int on_command_unquery(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = command_get_arg(cmd, 0);
    if (!nick){
        nick = srv->cur_chat->name;
    }

    return server_rm_chat(srv, nick);
}

static int on_command_join(Command *cmd, void *user_data){
    const char *chan;
    const char *passwd;
    Server *srv;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

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

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = scctx_get_chat(user_data);

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

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    reason = command_get_arg(cmd, 0);
    if (!reason) reason = PACKAGE_NAME " " PACKAGE_VERSION " " "quit.";

    return sirc_cmd_quit(srv->irc, reason);
}

static int on_command_topic(Command *cmd, void *user_data){
    const char *topic;
    Server *srv;
    Chat *chat;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = scctx_get_chat(user_data);
    g_return_val_if_fail(chat, SRN_ERR);

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

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_nick(srv->irc, nick);
}

static int on_command_whois(Command *cmd, void *user_data){
    const char *nick;
    Server *srv;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    nick = command_get_arg(cmd, 0);
    g_return_val_if_fail(nick, SRN_ERR);

    return sirc_cmd_whois(srv->irc, nick);
}

static int on_command_invite(Command *cmd, void *user_data){
    const char *nick;
    const char *chan;
    Server *srv;
    Chat *chat;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
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
    Chat *chat;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);
    chat = scctx_get_chat(user_data);

    nick = command_get_arg(cmd, 0);
    chan = command_get_arg(cmd, 1);
    reason = command_get_arg(cmd, 2);

    g_return_val_if_fail(nick, SRN_ERR);

    if (!chan && chat) {
        chan = chat->name;
    }

    return sirc_cmd_kick(srv->irc, nick, chan, reason);
}

static int on_command_mode(Command *cmd, void *user_data){
    const char *mode;
    const char *target;
    Server *srv;

    srv = scctx_get_server(user_data);
    g_return_val_if_fail(srv, SRN_ERR);

    target = command_get_arg(cmd, 0);
    mode = command_get_arg(cmd, 1);
    g_return_val_if_fail(target, SRN_ERR);
    g_return_val_if_fail(mode, SRN_ERR);

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

static void on_missing_arg(Command *cmd, int narg, void *user_data){
    Chat *chat;

    chat = scctx_get_chat(user_data);
    g_return_if_fail(chat);

    chat_add_error_message_fmt(chat, chat->user->nick,
            _("Missing arguments, expecting %d, actually %d"),
            cmd->bind->argc, narg);
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
