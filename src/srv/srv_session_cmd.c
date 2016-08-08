/**
 * @file srv_session_cmd.c
 * @brief Server comand parser
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-26
 */

#define __LOG_ON
// #define __DBG_ON

#include <string.h>
#include <strings.h>

#include "srv_session.h"
#include "srv_hdr.h"

#include "log.h"
#include "meta.h"
#include "i18n.h"
#include "filter.h"

#define IS_CMD(x, y) (strncasecmp(x, y, strlen(y)) == 0 && \
        (x[strlen(y)] == '\0' || x[strlen(y)] == ' '))

#define GET_PARAM(_params, _param, _var, _default) \
    do { \
        if (strncmp(_params, _param "=", sizeof(_param "=") - 1) == 0){ \
            _var = _params + sizeof(_param "=") - 1; \
            DBG_FR("GET_PARAM: %s: '%s'", _param, _var); \
            if (strlen(_var) == 0) _var = _default; \
        } \
    } while (0)

static void wait_until_connected(srv_session_t *session){
    DBG_FR("Waiting for session: %s", session->host);
    while (session->stat != SESS_CONNECT){
        sleep(1);
    };
    DBG_FR("Ready :)");
}


/**
 * @brief Execute a command
 *
 * @param session If null, the last used session will be used
 * @param source If source = NULL, fallback to SRV_SESSION_SERVER
 * @param cmd
 * @param block If block == 1 and `session`->stat != SESS_CONN, this function
 *              will blocked until `session`->stat == SESS_CONN
 *              If block == 0 and `session`->stat != SESS_CONN, command may not
 *              sent
 *
 * @return -1 if command fails
 */
int srv_session_cmd(srv_session_t *session, const char *source, char *cmd, int block){
    /* The last used session */
    static srv_session_t *last_sess = NULL;

    if (!source) source = SRV_SESSION_SERVER;

    /* Usage: /connect <host> <nick> [port=<port>,passwd=<passwd>,realname=<realname>,ssl=[on|noverify|off]] */
    if (IS_CMD(cmd, "/connect")){
        char *host = strtok(cmd + strlen("/connect"), " ");
        char *nick = strtok(NULL, " ");
        char *port = "0", *passwd = NULL, *realname = NULL, *ssl = "off";
        char *params = strtok(NULL, ",");
        while (params){
            GET_PARAM(params, "port", port, "0");
            GET_PARAM(params, "passwd", passwd, NULL);
            GET_PARAM(params, "realname", realname, PACKAGE_WEBSITE);
            GET_PARAM(params, "ssl", ssl, "off");
            params = strtok(NULL, ",");
        }

        if (!host || !nick) goto bad;

        ssl_opt_t sslopt = 0;
        if (strcmp(ssl, "on") == 0) sslopt = SSL_ON;
        if (strcmp(ssl, "off") == 0) sslopt = SSL_OFF;
        if (strcmp(ssl, "noverify") == 0) sslopt = SSL_NO_VERIFY;

        srv_session_t *tmp;
        tmp = srv_session_new(host, atoi(port), passwd, nick,
                PACKAGE_NAME, realname, sslopt);
        if (tmp){
            last_sess = tmp;
            return 0;
        } else {
            return -1;
        }
    }

    /* NB: relaybot parameters separated by '|' */
    else if (IS_CMD(cmd, "/relaybot")){
        char *bot = strtok(cmd + strlen("/relaybot"), " |");
        if (bot){
            DBG_FR("Relaybot: %s", bot);
            char *ldelim = strtok(NULL, "|");
            if (ldelim){
                DBG_FR("Left delim: '%s'", ldelim);
                char *rdelim = strtok(NULL, "|");
                if (rdelim){
                    DBG_FR("Right delim: '%s'", rdelim);
                    return filter_relaybot_list_add(bot, ldelim, rdelim);
                }
            }
        }
        goto bad;
    }

    /* Usage: /unrelaybot <nick> */
    else if (IS_CMD(cmd, "/unrelaybot")){
        char *nick = strtok(cmd + strlen("/unrelaybot"), " ");
        if (!nick) goto bad;
        return filter_relaybot_list_rm(nick);
    }

    /* Usage: /ignore <nick> */
    else if (IS_CMD(cmd, "/ignore")){
        char *nick = strtok(cmd + strlen("/ignore"), " ");
        if (!nick) goto bad;
        return filter_ignore_list_add(nick);
    }

    /* Usage: /unignore <nick> */
    else if (IS_CMD(cmd, "/unignore")){
        char *nick = strtok(cmd + strlen("/unignore"), " ");
        if (!nick) goto bad;
        return filter_ignore_list_rm(nick);
    }

    /* ================================================================
     * In the following commands, `source` and `session` MUST be vaild,
     * if session is NULL, use the last uesd session */
    if (!srv_session_is_session(session)) session = last_sess;
    if (!srv_session_is_session(session)) {
        ERR_FR("Session %p is invaild", session);
        return -1;
    }

    if (block) wait_until_connected(session);

    /* Usage: /query <nickname> */
    if (IS_CMD(cmd, "/query")){
        char *target = strtok(cmd + strlen("/query"), " ");
        if (IS_CHAN(target)) goto bad;
        srv_hdr_ui_add_chan(session->host, target);
        srv_session_whois(session, target);
    }

    /* Usage: /unquery [nickname] */
    else if (IS_CMD(cmd, "/unquery")){
        char *target = strtok(cmd + strlen("/unquery"), " ");
        if (!target) target = (char *)source;
        if (IS_CHAN(target)) goto bad;
        srv_hdr_ui_rm_chan(session->host, target);
    }

    /* Usage: /join <channel> [password] */
    else if (IS_CMD(cmd, "/join")){
        char *chan = strtok(cmd + strlen("/join"), " ");
        char *passwd = strtok(NULL, " ");
        if (!chan) goto bad;
        return srv_session_join(session, chan, passwd);
    }

    /* Usage: /part [channel] [reason] */
    else if (IS_CMD(cmd, "/part")){
        char *chan = strtok(cmd + strlen("/part"), " ");
        char *reason = strtok(NULL, "");
        if (!chan)
            chan = (char *)source;
        else if (!IS_CHAN(chan) && !reason){
            reason = chan;
            chan = NULL;
        }
        // FIXME: libircclent doesn't support part with reason?
        if (!chan) goto bad;
        return srv_session_part(session, chan);
    }

    /* Usage: /quit [reason] */
    else if (IS_CMD(cmd, "/quit")){
        char *reason = strtok(cmd + strlen("/quit"), " ");
        // FIXME: reason doesnt work
        return srv_session_quit(session, reason);
    }

    /* Usage: /msg <target> <message> */
    else if (IS_CMD(cmd, "/msg")){
        char *to = strtok(cmd + strlen("/msg"), " ");
        char *msg = strtok(NULL, "");
        if (!to || !msg) goto bad;
        srv_hdr_ui_send_msg(session->host, to, msg);
        return srv_session_send(session, to, msg);
    }

    /* Usage: /me <message> */
    else if (IS_CMD(cmd, "/me")){
        char *msg = strtok(cmd + strlen("/me"), "");
        char buf[512];
        if (!msg) goto bad;
        snprintf(buf, sizeof(buf), _("*** %s %s ***"), session->nickname, msg);
        srv_hdr_ui_sys_msg(session->host, source, buf, SYS_MSG_ACTION);
        return srv_session_me(session, source, msg);
    }

    /* Usage: /nick <new_nick> */
    else if (IS_CMD(cmd, "/nick")){
        char *nick = strtok(cmd + strlen("/nick"), " ");
        if (!nick) goto bad;
        return srv_session_nick(session, nick);

    }

    /* Usage: /whois [nick] */
    else if (IS_CMD(cmd, "/whois")){
        char *nick = strtok(cmd + strlen("/whois"), " ");
        if (nick == NULL){
            if (!IS_CHAN(source)) nick = (char *)source;
            else nick = session->nickname;
        }
        return srv_session_whois(session, nick);
    }

    /* Usage: /whois <nick> [channel] */
    else if (IS_CMD(cmd, "/invite")){
        char *nick = strtok(cmd + strlen("/invite"), " ");
        char *chan = strtok(NULL, " ");
        if (!chan && IS_CHAN(source)) chan = (char *)source;
        if (!nick || !chan) goto bad;
        return srv_session_invite(session, nick, chan);
    }

    /* Usage: /whois <nick> [channel] [reason] */
    else if (IS_CMD(cmd, "/kick")){
        char *nick = strtok(cmd + strlen("/kick"), " ");
        char *chan = strtok(NULL, " ");
        char *reason = strtok(NULL, "");
        if (!IS_CHAN(chan)){
            reason = chan;
            chan = (char *)source;
        }
        if (!nick && !chan) goto bad;
        return srv_session_kick(session, nick, chan, reason);
    }

    /* Usage: /mode <mode>
     *        /mode <mode> <channel>
     */
    else if (IS_CMD(cmd, "/mode")){
        char *mode = strtok(cmd + strlen("/mode"), " ");
        char *target = strtok(NULL, "");
        if (!mode) goto bad;
        return srv_session_mode(session, target, mode);
    }

    else if (IS_CMD(cmd, "/help")){
        srv_hdr_ui_sys_msg(session->host, source,
                _("Please visit" PACKAGE_WEBSITE "/wiki"),
                SYS_MSG_NORMAL);
    }

    /* no command matched */
    else {
        char buf[512];
        snprintf(buf, sizeof(buf), _("No such commmand: %s"), strtok(cmd, " "));
        srv_hdr_ui_sys_msg(session->host, source, buf, SYS_MSG_ERROR);
        return -1;
    }

    return 0;
bad:
    if (srv_session_is_session(session)){
        char buf[512];
        snprintf(buf, sizeof(buf), _("%s: wrong parameters"), cmd);
        srv_hdr_ui_sys_msg(session->host, source, buf, SYS_MSG_ERROR);
    } else {
        WARN_FR("Wrong parameters, session: %p, source: %s, cmd: %s",
                session, source, cmd);
    }
    return -1;
}
