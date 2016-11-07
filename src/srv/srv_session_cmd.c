/**
 * @file srv_session_cmd.c
 * @brief Server comand parser
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-26
 */

// #define __DBG_ON
#define __LOG_ON

#include <string.h>
#include <strings.h>

#include "srv_session.h"
#include "srv_hdr.h"

#include "log.h"
#include "meta.h"
#include "i18n.h"
#include "filter.h"
#include "command.h"

static srv_session_t *session = NULL;

static void wait_until_connected(){
    if (!session) return;

    DBG_FR("Waiting for session: %s ...", session->host);
    while (session->stat != SESS_CONNECT){
        sleep(1);
    };
}

static int on_command_connect(Command *cmd, void *user_data){
    const char *host = command_get_arg(cmd, 0);
    const char *nick = command_get_arg(cmd, 1);
    char *port, *passwd, *ssl, *realname;
    command_get_opt(cmd, "-port", &port);
    command_get_opt(cmd, "-passwd", &passwd);
    command_get_opt(cmd, "-ssl", &ssl);
    command_get_opt(cmd, "-realname", &realname);

    if (!host || !nick) return -1;
    ssl_opt_t sslopt = SSL_OFF;
    if (strcmp(ssl, "on") == 0) sslopt = SSL_ON;
    if (strcmp(ssl, "off") == 0) sslopt = SSL_OFF;
    if (strcmp(ssl, "noverify") == 0) sslopt = SSL_NO_VERIFY;

    if (*passwd == '\0') passwd = NULL;

    srv_session_t *tmp;
    tmp = srv_session_new(host, atoi(port), passwd, nick,
            PACKAGE_NAME, realname, sslopt);
    if (tmp){
        session = tmp;
        return 0;
    } else {
        return -1;
    }
}

static int on_command_relay(Command *cmd, void *user_data){
    char *ldelim, *rdelim;
    const char *nick = command_get_arg(cmd, 0);
    command_get_opt(cmd, "-ldelim", &ldelim);
    command_get_opt(cmd, "-rdelim", &rdelim);

    if (!nick) return -1;
    return filter_relaybot_list_add(nick, ldelim, rdelim);
}

static int on_command_unrelay(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    return filter_relaybot_list_rm(nick);
}

static int on_command_ignore(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    return filter_ignore_list_add(nick);
}

static int on_command_unignore(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    return filter_ignore_list_rm(nick);
}

static int on_command_query(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    srv_hdr_ui_add_chat(session->host, nick, "", CHAT_PRIVATE);
    srv_session_whois(session, nick);
    return 0;
}

static int on_command_unquery(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    srv_hdr_ui_rm_chat(session->host, nick);
    return 0;
}

static int on_command_join(Command *cmd, void *user_data){
    const char *chan = command_get_arg(cmd, 0);
    const char *passwd = command_get_arg(cmd, 1);

    if (!chan) return -1;
    return srv_session_join(session, chan, passwd);
}

static int on_command_part(Command *cmd, void *user_data){
    const char *chan = command_get_arg(cmd, 0);
    // const char *reason = command_get_arg(cmd, 1);

    if (!chan) chan = user_data;
    return srv_session_part(session, chan);
}

static int on_command_quit(Command *cmd, void *user_data){
    const char *reason = command_get_arg(cmd, 0);

    return srv_session_quit(session, reason);
}

static int on_command_topic(Command *cmd, void *user_data){
    const char *channel = user_data;
    const char *topic = command_get_arg(cmd, 0);
    if (command_get_opt(cmd, "-rm", NULL)) topic = "";

    return srv_session_topic(session, channel, topic);
}

static int on_command_msg(Command *cmd, void *user_data){
    const char *target = command_get_arg(cmd, 0);
    const char *msg = command_get_arg(cmd, 1);

    if (!target || !msg) return -1;
    srv_hdr_ui_send_msg(session->host, target, msg);
    return srv_session_send(session, target, msg);
}

static int on_command_me(Command *cmd, void *user_data){
    const char *source = user_data;
    const char *msg = command_get_arg(cmd, 0);
    if (!msg) return -1;

    char buf[512];
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), session->nickname, msg);
    srv_hdr_ui_sys_msg(session->host, source, buf, SYS_MSG_ACTION);
    return srv_session_me(session, source, msg);
}

static int on_command_nick(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    return srv_session_nick(session, nick);
}

static int on_command_whois(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);

    if (!nick) return -1;
    return srv_session_whois(session, nick);
}

static int on_command_invite(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    const char *chan = command_get_arg(cmd, 1);

    if (!nick) return -1;
    if (!chan) chan = user_data;
    return srv_session_whois(session, nick);
}

static int on_command_kick(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    const char *chan = command_get_arg(cmd, 1);
    const char *reason = command_get_arg(cmd, 3);

    if (!nick || !chan) return -1;
    return srv_session_kick(session, nick, chan, reason);
}

static int on_command_mode(Command *cmd, void *user_data){
    const char *mode = command_get_arg(cmd, 0);
    const char *target = command_get_arg(cmd, 1);

    if (!target || !mode) return -1;
    return srv_session_mode(session, target, mode);

}

static int on_command_list(Command *cmd, void *user_data){
    return 0;
}

static CommandBind cmd_binds[] = {
    {
        "/connect", 2, // <hosts> <nick>
        {"-port", "-ssl", "-passwd", "-realname", NULL},
        {"6667", "off", "", "Can you can a can?", NULL},
        on_command_connect
    },
    {
        "/relay", 1, // <nick>
        {"-ldelim", "-rdelim", NULL},
        {"", "", NULL},
        on_command_relay
    },
    {
        "/unrelay", 1, // <nick>
        {NULL},
        {NULL},
        on_command_unrelay
    },
    {
        "/ignore", 1, // <nick>
        {NULL},
        {NULL},
        on_command_ignore
    },
    {
        "/unignore", 1, // <nick>
        {NULL},
        {NULL},
        on_command_unignore
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

void srv_session_cmd_init(){
    commmad_bind(cmd_binds);
}

int srv_session_cmd(srv_session_t *sess, const char *source, char *cmd, int block){
    if (source == NULL) source = META_SERVER;
    if (sess) session = sess;
    wait_until_connected();
    return command_proc(cmd, (char *)source);
}
