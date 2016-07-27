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

#include "srv.h"
#include "srv_session.h"
#include "srv_hdr.h"

#include "log.h"

#define IS_CMD(x, y) (strncasecmp(x, y, strlen(y)) == 0 && \
        (x[strlen(y)] == '\0' || x[strlen(y)] == ' '))

int srv_session_cmd(srv_session_t *session, const char *source, char *cmd){

    /* /connect <host>:[port] ... */
    if (IS_CMD(cmd, "/connect")){
        char *host = strtok(cmd + strlen("/connect"), " ");
        if (host){
            return srv_connect(host, 6667, NULL, "srainbot", NULL, NULL);
        }
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

     /********************************************
      * in the following commands,
      * `chan_name` and `srv` **CANNOT** be NULL,
      * so, check it.
      ********************************************/
    else if (session == NULL || source == NULL){
        ERR_FR("chan_name or server shouldn't be NULL");
        return -1;
    }

    else if (IS_CMD(cmd, "/login")){
        char *nick = strtok(cmd + strlen("/login"), " ");
        // if (nick) return server_login(srv, nick);
    }


    else if (IS_CMD(cmd, "/query")){
        char *target = strtok(cmd + strlen("/query"), " ");
        // if (target) return server_query(srv, target);
    }

    else if (IS_CMD(cmd, "/unquery")){
        char *target = strtok(cmd + strlen("/unquery"), " ");
        // if (target == NULL) target = (char *)chan_name;
            // return server_unquery(srv, target);
    }

    else if (IS_CMD(cmd, "/join")){
        char *jchan = strtok(cmd + strlen("/join"), " ");
        if (jchan) return srv_session_join(session, jchan, 0);
    }

    else if (IS_CMD(cmd, "/part")){
        char *pchan = strtok(cmd + strlen("/part"), " ");
        if (pchan == NULL) pchan = (char *)source;
        return srv_session_part(session, pchan);
        // TODO reason
    }

    else if (IS_CMD(cmd, "/quit")){
        srv_session_quit(session, "Quit");
        return 0;
    }

    else if (IS_CMD(cmd, "/msg")){
        char *to = strtok(cmd + strlen("/msg"), " ");
        char *msg = strtok(NULL, "");
        if (to && msg) return srv_session_send(session, to, msg);
    }

    else if (IS_CMD(cmd, "/me")){
        char *msg = cmd + 4;
        if (msg){
            srv_hdr_ui_sys_msg(session->host, source, msg, SYS_MSG_NORMAL);
            return srv_session_me(session, source, msg);
        }
    }

    else if (IS_CMD(cmd, "/nick")){
        char *nick = strtok(cmd + strlen("/nick"), " ");
        if (nick){
            /* irc->nick will be modified when recv
             * NICK command from server */
            // return irc_nick_req(&(srv->irc), nick);
        }
    }

    else if (IS_CMD(cmd, "/whois")){
        char *nick = strtok(cmd + strlen("/whois"), " ");
        // if (nick == NULL) nick = (char *)chan_name;
        // return irc_whois(&(srv->irc), nick);
    }

    else if (IS_CMD(cmd, "/invite")){
        char *nick = strtok(cmd + strlen("/invite"), " ");
        char *ichan = strtok(NULL, " ");
        if (nick){
            // server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_NORMAL,
                    // "You have invited %s to %s", nick, ichan);
            // return irc_invite(&(srv->irc), nick, ichan);
        }
    }

    else if (IS_CMD(cmd, "/kick")){
        char *nick = strtok(cmd + strlen("/kick"), " ");
        char *kchan = strtok(NULL, " ");
        char *reason = strtok(NULL, "");
        if (nick){
            if (reason == NULL) reason = "";
            // return irc_kick(&(srv->irc), nick, kchan, reason);
        }
    }

    else if (IS_CMD(cmd, "/mode")){
        char *target = strtok(cmd + strlen("/mode"), " ");
        char *mode = strtok(NULL, "");
        if (target){
            if (mode == NULL) mode = "";
            // return irc_mode(&(srv->irc), target, mode);
        }
    }

    else if (IS_CMD(cmd, "/help")){
        // TODO: remove it?
        // server_intf_ui_sys_msg(srv, chan_name, help, SYS_MSG_NORMAL);
        return 0;
    }

    /* no command matched */
    else {
        // (srv, chan_name, SYS_MSG_ERROR,
                // "%s: unsupported command", cmd);
        return -1;
    }

    return 0;
}

