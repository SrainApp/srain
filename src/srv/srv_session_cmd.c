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

typedef struct {
    SRVSession *session;
    const char *source;
} SessionCommandContext;

static SRVSession *startup_session;
static int on_command_connect(Command *cmd, void *user_data);
static int on_command_relay(Command *cmd, void *user_data);
static int on_command_unrelay(Command *cmd, void *user_data);
static int on_command_ignore(Command *cmd, void *user_data);
static int on_command_filter(Command *cmd, void *user_data);
static int on_command_filter_show(Command *cmd, void *user_data);
static int on_command_unfilter(Command *cmd, void *user_data);
static int on_command_unignore(Command *cmd, void *user_data);
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
        "/filter", 3, // <filter name> <channel> <regex>
        {NULL},
        {NULL},
        on_command_filter
    },
    {
        "/unfilter", 1, // <filter name>
        {NULL},
        {NULL},
        on_command_unfilter
    },
    {
        "/query", 1, // <nick>
        {NULL},
        {NULL},
        on_command_query
    },
    {
        "/filterlist", 0,
        {NULL},
        {NULL},
        on_command_filter_show
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

static CommandContext cmd_context = {
    .binds = cmd_binds,
    .on_unknown_cmd = on_unknown_cmd,
    .on_unknown_opt = on_unknown_opt,
    .on_missing_opt_val = on_missing_opt_val,
    .on_too_many_opt = on_too_many_opt,
    .on_too_many_arg = on_too_many_arg,
    .on_callback_fail = on_callback_fail,
};

static int on_command_connect(Command *cmd, void *user_data){
    const char *host = command_get_arg(cmd, 0);
    const char *nick = command_get_arg(cmd, 1);
    char *port, *passwd, *ssl, *realname;

    command_get_opt(cmd, "-port", &port);
    command_get_opt(cmd, "-passwd", &passwd);
    command_get_opt(cmd, "-ssl", &ssl);
    command_get_opt(cmd, "-realname", &realname);

    if (!host || !nick) return -1;
    SRVSessionFlag flag = 0;
    if (strcmp(ssl, "on") == 0) flag |= SRV_SESSION_FLAG_SSL;
    if (strcmp(ssl, "noverify") == 0){
        flag |= SRV_SESSION_FLAG_SSL | SRV_SESSION_FLAG_SSL_NOVERIFY;
    }

    if (*passwd == '\0') passwd = NULL;

    SRVSession *tmp;
    tmp = srv_session_new(host, atoi(port), passwd, nick,
            PACKAGE_NAME, realname, flag);
    if (tmp){
        startup_session = tmp;
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

static int on_command_filter(Command *cmd, void *user_data){
    const char *name = command_get_arg(cmd, 0);
    const char *channel_name = command_get_arg(cmd, 1);
    const char *regex = command_get_arg(cmd, 2);

    if(!name || !channel_name || !regex)
        return -1;
    return filter_filter_add_filter(name, regex, channel_name);
}

static int on_command_unfilter(Command *cmd, void *user_data){
    const char *name = command_get_arg(cmd, 0);
    if (!name) return -1;

    return filter_filter_remove_filter(name);
}

static int on_command_filter_show(Command *cmd, void *user_data){
    return filter_filter_show();
}

static int on_command_query(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    if (!nick) return -1;
    srv_hdr_ui_add_chat(ctx->session->host, nick, "", CHAT_PRIVATE);
    srv_session_whois(ctx->session, nick);
    return 0;
}

static int on_command_unquery(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    if (!nick) return -1;
    srv_hdr_ui_rm_chat(ctx->session->host, nick);
    return 0;
}

static int on_command_join(Command *cmd, void *user_data){
    const char *chan = command_get_arg(cmd, 0);
    const char *passwd = command_get_arg(cmd, 1);
    SessionCommandContext *ctx = user_data;


    if (!chan) return -1;
    return srv_session_join(ctx->session, chan, passwd);
}

static int on_command_part(Command *cmd, void *user_data){
    const char *chan = command_get_arg(cmd, 0);
    // const char *reason = command_get_arg(cmd, 1);
    SessionCommandContext *ctx = user_data;


    if (!chan) chan = user_data;
    return srv_session_part(ctx->session, chan);
}

static int on_command_quit(Command *cmd, void *user_data){
    const char *reason = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    return srv_session_quit(ctx->session, reason);
}

static int on_command_topic(Command *cmd, void *user_data){
    const char *topic = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    if (command_get_opt(cmd, "-rm", NULL)) topic = "";
    return srv_session_topic(ctx->session, ctx->source, topic);
}

static int on_command_msg(Command *cmd, void *user_data){
    const char *target = command_get_arg(cmd, 0);
    const char *msg = command_get_arg(cmd, 1);
    SessionCommandContext *ctx = user_data;

    if (!target || !msg) return -1;
    srv_hdr_ui_send_msg(ctx->session->host, target, msg, 0);
    return srv_session_send(ctx->session, target, msg);
}

static int on_command_me(Command *cmd, void *user_data){
    const char *msg = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    if (!msg) return -1;

    char buf[512];
    snprintf(buf, sizeof(buf), _("*** %s %s ***"), ctx->session->nickname, msg);
    srv_hdr_ui_sys_msg(ctx->session->host, ctx->source, buf, SYS_MSG_ACTION, 0);
    return srv_session_me(ctx->session, ctx->source, msg);
}

static int on_command_nick(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    if (!nick) return -1;
    return srv_session_nick(ctx->session, nick);
}

static int on_command_whois(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    SessionCommandContext *ctx = user_data;

    if (!nick) return -1;
    return srv_session_whois(ctx->session, nick);
}

static int on_command_invite(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    const char *chan = command_get_arg(cmd, 1);
    SessionCommandContext *ctx = user_data;

    if (!nick) return -1;
    if (!chan) chan = ctx->source;
    return srv_session_invite(ctx->session, ctx->source, nick);
}

static int on_command_kick(Command *cmd, void *user_data){
    const char *nick = command_get_arg(cmd, 0);
    const char *chan = command_get_arg(cmd, 1);
    const char *reason = command_get_arg(cmd, 3);
    SessionCommandContext *ctx = user_data;

    if (!nick || !chan) return -1;
    return srv_session_kick(ctx->session, nick, chan, reason);
}

static int on_command_mode(Command *cmd, void *user_data){
    const char *mode = command_get_arg(cmd, 0);
    const char *target = command_get_arg(cmd, 1);
    SessionCommandContext *ctx = user_data;

    if (!target || !mode) return -1;
    return srv_session_mode(ctx->session, target, mode);

}

static int on_command_list(Command *cmd, void *user_data){
    return 0;
}

static void on_unknown_cmd(const char *cmd, void *user_data){
    SessionCommandContext *ctx = user_data;

    srv_hdr_ui_sys_msgf(ctx->session->host, ctx->source, SYS_MSG_ERROR, 0,
            _("No such command '%s'"), cmd);
}

static void on_unknown_opt(Command *cmd, const char *opt, void *user_data){
    SessionCommandContext *ctx = user_data;

    srv_hdr_ui_sys_msgf(ctx->session->host, ctx->source, SYS_MSG_ERROR, 0,
            _("No such option '%s'"), opt);
}

static void on_missing_opt_val(Command *cmd, const char *opt, void *user_data){
    SessionCommandContext *ctx = user_data;

    srv_hdr_ui_sys_msgf(ctx->session->host, ctx->source, SYS_MSG_ERROR, 0,
            _("Option '%s' missing value"), opt);
}

static void on_too_many_opt(Command *cmd, void *user_data){
    SessionCommandContext *ctx = user_data;

    srv_hdr_ui_sys_msg(ctx->session->host, ctx->source, _("Too many options"),
            SYS_MSG_ERROR, 0);
}

static void on_too_many_arg(Command *cmd, void *user_data){
    SessionCommandContext *ctx = user_data;

    srv_hdr_ui_sys_msg(ctx->session->host, ctx->source, _("Too many arguments"),
            SYS_MSG_ERROR, 0);
}

static void on_callback_fail(Command *cmd, void *user_data){
    SessionCommandContext *ctx = user_data;

    srv_hdr_ui_sys_msgf(ctx->session->host, ctx->source, SYS_MSG_ERROR, 0,
            _("Command '%s' failed"), cmd->bind->name);
}

static void wait_until_connected(SRVSession *session){
    if (!session) return;

    DBG_FR("Waiting for session: %s ...", session->host);
    while (session->stat != SRV_SESSION_STAT_CONNECT){
        sleep(1);
    };
}

void srv_session_cmd_init(){
    commmad_set_context(&cmd_context);
}

int srv_session_cmd(SRVSession *session, const char *source, char *cmd, int block){
    SessionCommandContext ctx;

    if (source == NULL) source = META_SERVER;
    if (session == NULL) session = startup_session;
    if (block) wait_until_connected(session);

    ctx.session = session;
    ctx.source = source;

    return command_proc(cmd, &ctx);
}
/* vim: set et ts=4 sw=4 sts=4: */
