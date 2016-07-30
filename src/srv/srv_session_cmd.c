/**
 * @file srv_session_cmd.c
 * @brief Server comand parser
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-26
 */

#define __LOG_ON
#define __DBG_ON

#include <string.h>
#include <strings.h>

#include "srv_session.h"
#include "srv_hdr.h"

#include "meta.h"
#include "log.h"
#include "i18n.h"

#define IS_CMD(x, y) (strncasecmp(x, y, strlen(y)) == 0 && \
        (x[strlen(y)] == '\0' || x[strlen(y)] == ' '))

#define GET_PARAM(_params, _param, _var, _default) \
    do { \
        if (strncmp(_params, _param "=", sizeof(_param "=") - 1) == 0){ \
            _var = _params + sizeof(_param "=") - 1; \
            DBG_FR("GET_PARAM: %s: %s", _param, _var); \
            if (strlen(_var) == 0) _var = _default; \
        } \
    } while (0)

int srv_session_cmd(srv_session_t *session, const char *source, char *cmd){

    WARN_FR("Wrong parameters, session: %p source: %s, cmd: %s", session, source, cmd);
    if (session) WARN_FR("stat: %d", session->stat);
    /* Usage: /connect <host> <nick> [port=<port>] [passwd=<passwd>] [realname=<realname>] */
    if (IS_CMD(cmd, "/connect")){
        char *host = strtok(cmd + strlen("/connect"), " ");
        char *nick = strtok(NULL, " ");
        char *port = "0", *passwd = NULL, *realname = NULL;
        char *params = strtok(NULL, " ");
        while (params){
            GET_PARAM(params, "port", port, "0");
            GET_PARAM(params, "passwd", passwd, NULL);
            GET_PARAM(params, "realname", passwd, PACKAGE_WEBSITE);
            LOG_FR("params %s", params);
            params = strtok(NULL, " ");
        }

        if (!host || !nick) goto bad;
        return srv_session_new(host, atoi(port), passwd, nick,
                PACKAGE_VERSION, realname) ? 0 : -1;
    }

    /* NB: relaybot parameters separated by '|' */
    else if (IS_CMD(cmd, "/relaybot")){
        char *bot = strtok(cmd + strlen("/relaybot"), " |");
        if (bot){
            char *ldelim = strtok(NULL, "|");
            if (ldelim){
                char *rdelim = strtok(NULL, "|");
                if (rdelim){
                    // TODO
                    // return filter_relaybot_list_add(bot, ldelim, rdelim);
                }
            }
        }
    }

    else if (IS_CMD(cmd, "/ignore")){
        char *nick = strtok(cmd + strlen("/ignore"), " ");
        // TODO
        // if (nick) return filter_ignore_list_add(nick);
    }
    // TODO: impl /unignore
    // TODO: impl /unrelaybot

    if (session) WARN_FR("stat: %d", session->stat);
    /* In the following commands, `source` and `session` MUST be vaild */
    if (srv_session_is_session(session) && source){

        /* Usage: /query <nickname> */
        if (IS_CMD(cmd, "/query")){
            char *target = strtok(cmd + strlen("/query"), " ");
            if (IS_CHAN(target)) goto bad;
            srv_hdr_ui_add_chan(session->host, target);
        }

        /* Usage: /unquery [nickname] */
        else if (IS_CMD(cmd, "/unquery")){
            char *target = strtok(cmd + strlen("/unquery"), " ");
            if (!target) target = (char *)source;
            if (IS_CHAN(target)) goto bad;
            srv_hdr_ui_add_chan(session->host, target);
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
            char *msg = strtok(cmd + strlen("/me"), " ");
            if (!msg) goto bad;
            srv_hdr_ui_sys_msg(session->host, source, msg, SYS_MSG_NORMAL);
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
                    _("Please visit " PACKAGE_WEBSITE "/wiki"),
                    SYS_MSG_NORMAL);
        }

        /* no command matched */
        else {
            WARN_FR("No such command, session: %s, source: %s, cmd: %s",
                    session->host, source, cmd);
            srv_hdr_ui_sys_msg(session->host, source,
                    _("No such command"), SYS_MSG_ERROR);
            goto bad;
        }

        return 0;
    }

    ERR_FR("session: %p, source: %s, one of them is invaild", session, source);
    return -1;

bad:
    if (srv_session_is_session(session) && source){
        srv_hdr_ui_sys_msg(session->host, source,
                _("Wrong parameters"), SYS_MSG_ERROR);
    } else {
        WARN_FR("Wrong parameters, session: %p, source: %s, cmd: %s",
                session, source, cmd);
    }
    return -1;
}
