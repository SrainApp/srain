#define __LOG_ON

#include <strings.h>
#include "meta.h"
#include "server.h"
#include "filter.h"
#include "server_intf.h"
#include "log.h"

#define IS_CMD(x, y) (strncasecmp(x, y, strlen(y)) == 0 && \
        (x[strlen(y)] == '\0' || x[strlen(y)] == ' '))

/**
 * @brief server_cmd send a command to server
 *
 * @param srv server
 * @param chan_name where the `cmd` comes from,
 * @param cmd command
 *
 * @return 0 if successed, -1 if failed
 */
int server_cmd(IRCServer *srv, const char *chan_name, char *cmd){
    LOG_FR("server: %s, chan: %s, cmd: '%s'",
            srv ? srv->host : "(null)", chan_name, cmd);

    if (IS_CMD(cmd, "/help")){
        static char help[] = META_CMD_HELP;
        // TODO: remove it?
        server_intf_ui_sys_msg(srv, chan_name, help, SYS_MSG_NORMAL);
        return 0;
    }
    else if (IS_CMD(cmd, "/connect")){
        char *host = strtok(cmd + strlen("/connect"), " ");
        if (host){
            return server_connect(host) ? 0 : -1;
        }
    }
    else if (IS_CMD(cmd, "/login")){
        char *nick = strtok(cmd + strlen("/login"), " ");
        if (nick) return server_login(srv, nick);
    }
    /* NB: relaybot parameters separated by '|' */
    else if (IS_CMD(cmd, "/relaybot")){
        char *bot = strtok(cmd + strlen("/relaybot"), " |");
        if (bot){
            char *ldelim = strtok(NULL, "|");
            if (ldelim){
                char *rdelim = strtok(NULL, "|");
                if (rdelim){
                    return filter_relaybot_list_add(bot, ldelim, rdelim);
                }
            }
        }
    }
    else if (IS_CMD(cmd, "/ignore")){
        char *nick = strtok(cmd + strlen("/ignore"), " ");
        if (nick) return filter_ignore_list_add(nick);
    }

    else if (IS_CMD(cmd, "/query")){
        char *target = strtok(cmd + strlen("/query"), " ");
        if (target) return server_query(srv, target);
    }
    else if (IS_CMD(cmd, "/unquery")){
        char *target = strtok(cmd + strlen("/unquery"), " ");
        if (target == NULL) target = (char *)chan_name;
            return server_unquery(srv, target);
    }
    else if (IS_CMD(cmd, "/join")){
        char *jchan = strtok(cmd + strlen("/join"), " ");
        if (jchan) return server_join(srv, jchan);
    }
    else if (IS_CMD(cmd, "/part")){
        char *pchan = strtok(cmd + strlen("/part"), " ");
        if (pchan == NULL) pchan = (char *)chan_name;
        return server_part(srv, pchan, NULL);
    }
    else if (IS_CMD(cmd, "/quit")){
        server_close(srv);
        return 0;
    }
    else if (IS_CMD(cmd, "/msg")){
        char *to = strtok(cmd + strlen("/msg"), " ");
        char *msg = strtok(NULL, "");
        if (to && msg) return server_send(srv, to, msg);
    }
    else if (IS_CMD(cmd, "/me")){
        char *msg = cmd + 4;
        if (msg){
            server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_ACTION,
                    "*** %s %s ***", srv->irc.nick, msg);
            return irc_send(&(srv->irc), chan_name, msg, 1);
        }
    }
    else if (IS_CMD(cmd, "/nick")){
        char *nick = strtok(cmd + strlen("/nick"), " ");
        if (nick){
            /* irc->nick will be modified when recv
             * NICK command from server */
            return irc_nick_req(&(srv->irc), nick);
        }
    }
    else if (IS_CMD(cmd, "/whois")){
        char *nick = strtok(cmd + strlen("/whois"), " ");
        if (nick == NULL) nick = (char *)chan_name;
        return irc_whois(&(srv->irc), nick);
    }
    else if (IS_CMD(cmd, "/invite")){
        char *nick = strtok(cmd + strlen("/invite"), " ");
        char *ichan = strtok(NULL, " ");
        if (nick){
            server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_NORMAL,
                    "You have invited %s to %s", nick, ichan);
            return irc_invite(&(srv->irc), nick, ichan);
        }
    }
    else if (IS_CMD(cmd, "/kick")){
        char *nick = strtok(cmd + strlen("/kick"), " ");
        char *kchan = strtok(NULL, " ");
        char *reason = strtok(NULL, "");
        if (nick){
            if (reason == NULL) reason = "";
            return irc_kick(&(srv->irc), nick, chan_name, reason);
        }
    }
    else if (IS_CMD(cmd, "/mode")){
        char *target = strtok(cmd + strlen("/mode"), " ");
        char *mode = strtok(NULL, "");
        if (target){
            if (mode == NULL) mode = "";
            return irc_mode(&(srv->irc), target, mode);
        }
    } else {
        server_intf_ui_sys_msgf(srv, chan_name, SYS_MSG_ERROR,
                "%s: unsupported command", cmd);
        return -1;
    }

    server_intf_ui_sys_msg(srv, chan_name, "Missing parameter", SYS_MSG_ERROR);
    return -1;
}
