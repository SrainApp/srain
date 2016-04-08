/**
 * @file srain.c
 * @brief logic control layer of the whole application
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * This file gules UI module and IRC module together
 * and provides some abstract operations of a IRC client:
 *      connect to server, login as someone, join a channel,
 *      part from a channel, send message, receive message and etc.
 *
 */

#define __LOG_ON

#include <string.h>
#include <gtk/gtk.h>
#include "meta.h"
#include "i18n.h"
#include "irc.h"
#include "irc_magic.h"
#include "ui.h"
#include "srain_window.h"
#include "srain_msg.h"
#include "log.h"
#include "markup.h"
#include "filter.h"

// return and stop spinner: call ui_busy(FALSE)
#define RET(x) do { int tmp = x; ui_busy(FALSE); return tmp; } while (0)

irc_t irc;
GThread *thread = NULL;

enum {
    SRAIN_UNCONNECTED,
    SRAIN_CONNECTING,
    SRAIN_CONNECTED,
    SRAIN_LOGINED
} stat = SRAIN_UNCONNECTED;

void srain_recv();
void _srain_connect(const char *server){
    if (stat != SRAIN_UNCONNECTED){
        ERR_FR("you have connected");
        return;
    }

    stat = SRAIN_CONNECTING;
    if (irc_connect(&irc, server, "6666") == 0){
        stat = SRAIN_CONNECTED;
        srain_recv();   // endless loop
    } else {
        stat = SRAIN_UNCONNECTED;
    }
}

int srain_connect(const char *server){
    ui_busy(TRUE);

    if (!thread) {
        thread = g_thread_new(NULL, (GThreadFunc)_srain_connect, (char *)server);

        ui_chan_set_topic(META_SERVER, _("connecting..."));
        ui_msg_sysf(META_SERVER, SYS_MSG_NORMAL, _("connecting to %s ..."), server);

        while (stat == SRAIN_CONNECTING)
            while (gtk_events_pending()) gtk_main_iteration();

        if (stat == SRAIN_CONNECTED){
            ui_chan_set_topic(META_SERVER, server);
            RET(0);
        } else {
            ui_chan_set_topic(META_SERVER, _("connection failed"));
            ui_msg_sys(META_SERVER, SYS_MSG_ERROR, "connection failed");
            RET(-1);
        }

    }

    ui_msg_sys(META_SERVER, SYS_MSG_ERROR, "you have connected");
    RET(-1);
}

int srain_login(const char *nick){
    ui_busy(TRUE);

    if (stat != SRAIN_CONNECTED){
        ui_msg_sys(NULL, SYS_MSG_ERROR, "no connected");
        ERR_FR("NO SRAIN_CONNECTED");
        RET(-1);
    }

    if (irc_login(&irc, nick) >= 0){
        stat = SRAIN_LOGINED;
        RET(0);
    }

    RET(-1);
}

int srain_join(const char *chan){
    ui_busy(TRUE);

    if (stat != SRAIN_LOGINED){
        ui_msg_sys(NULL, SYS_MSG_ERROR, "no logged in");
        ERR_FR("NO SRAIN_LOGINED");
        RET(-1);
    }

    RET(irc_join_req(&irc, chan));
}

int srain_part(const char *chan, const char *reason){
    ui_busy(TRUE);

    if (!reason) reason = META_NAME_VERSION;

    RET(irc_part_req(&irc, chan, reason));
}

int srain_query(const char *target){
    ui_busy(TRUE);

    if (stat != SRAIN_LOGINED){
        ui_msg_sys(NULL, SYS_MSG_ERROR, "no logged in");
        ERR_FR("NO SRAIN_LOGINED");
        RET(-1);
    }

    if (IS_CHAN(target)){
        RET(srain_join(target));
    }

    ui_chan_add(target);

    RET(0);
}

int srain_unquery(const char *target){
    ui_busy(TRUE);

    if (stat != SRAIN_LOGINED){
        ui_msg_sys(NULL, SYS_MSG_ERROR, "no logged in");
        ERR_FR("NO SRAIN_LOGINED");
        RET(-1);
    }

    if (IS_CHAN(target)){
        RET(srain_part(target, NULL));
    }

    ui_chan_rm(target);

    RET(0);
}

int srain_send(const char *chan, char *msg){
    ui_busy(TRUE);

    LOG_FR("send message '%s' to %s", msg, chan);


    ui_msg_send(chan, msg);

    if (irc_send(&irc, chan, msg, 0) <= 0){
        ui_msg_sysf(NULL, SYS_MSG_ERROR, "faild to send message \"%.8s...\"", msg);
        RET(-1);
    }

    RET(0);
}

/* GSourceFunc */
gboolean srain_idles(irc_msg_t *imsg){
    /* Message */
    if (strcmp(imsg->command, "PRIVMSG") == 0){
        int is_action = 0;
        int imsg_len;
        const char *dest;

        if (imsg->nparam != 1) goto bad;

        imsg_len = strlen(imsg->message);

        /* strip '\x1ACTION '... '\x1' */
        if (strcmp(imsg->message, "\1ACTION ")
                && imsg->message[imsg_len - 1] == '\1'){
            char tmp_msg[MSG_LEN];

            is_action = 1;
            imsg->message[imsg_len - 1] = '\0';

            strncpy(tmp_msg, imsg->message + strlen("\1ACTION "),
                    MSG_LEN - strlen("\1ACTION "));
            memset(imsg->message, 0, MSG_LEN);
            strncpy(imsg->message, tmp_msg, MSG_LEN);
        }

        memset(imsg->servername, 0, SERVER_LEN);
        filter_relaybot_trans(imsg);

        if (filter_is_ignore(imsg->nick)){
            free(imsg);
            return FALSE;
        }

        /* when a message comes from a channel,
         * param[0] is channel's name
         * when it comes from a person,
         * param[0] yourself's nick
         */
        if (IS_CHAN(imsg->param[0])){
            dest = imsg->param[0];
        } else {
            dest = imsg->nick;
        }

        if (is_action){
            /* may lose relay bot information */
            ui_msg_sysf(dest, SYS_MSG_ACTION, "*** %s %s ***",
                    imsg->nick, imsg->message);
        } else {
            ui_msg_recv(dest, imsg->nick, imsg->servername, imsg->message);
        }
    }

    /* Topic */
    else if (strcmp(imsg->command, "TOPIC") == 0){
        if (imsg->nparam != 1) goto bad;
        ui_chan_set_topic(imsg->param[0], imsg->message);
    }
    else if (strcmp(imsg->command, RPL_TOPIC) == 0){
        if (imsg->nparam == 2) ui_chan_set_topic(imsg->param[1], imsg->message);
    }

    /* JOIN & PART & QUIT
     *
     * sys message "xxx has join/leave #yyy" are sent
     * by ui_chan_user_list_{add,rm}()
     */
    else if (strcmp(imsg->command, "JOIN") == 0){
        if (imsg->nparam != 1) goto bad;
        if (strncmp(imsg->nick, irc.nick, NICK_LEN) == 0){
            ui_chan_add(imsg->param[0]);
            irc_join_ack(&irc, imsg->param[0]);
        }
        ui_chan_user_list_add(imsg->param[0], imsg->nick,
                IRC_USER_PERSON, 1);
    }
    else if (strcmp(imsg->command, "PART") == 0){
        if (imsg->nparam != 1) goto bad;
        ui_chan_user_list_rm(imsg->param[0], imsg->nick, imsg->message);
        if (strncmp(imsg->nick, irc.nick, NICK_LEN) == 0){
            irc_part_ack(&irc, imsg->param[0]);
            ui_chan_rm(imsg->param[0]);
        }
    }
    else if (strcmp(imsg->command, "QUIT") == 0){
        if (imsg->nparam != 0) goto bad;
        ui_chan_user_list_rm_broadcast(irc.chans, imsg->nick, imsg->message);
    }

    /* INVITE & KICK */
    else if (strcmp(imsg->command, "INVITE") == 0){
        if (imsg->nparam != 1) goto bad;
        ui_msg_sysf(NULL, SYS_MSG_NORMAL, "%s invites you into %s",
                imsg->nick, imsg->message);
        goto bad;
    }
    else if (strcmp(imsg->command, "KICK") == 0){
        if (imsg->nparam != 2) goto bad;
        ui_msg_sysf(imsg->param[0], SYS_MSG_ERROR, "%s are kicked from %s by %s",
                imsg->param[1], imsg->param[0], imsg->nick);
    }

    /* NICK (someone change his name) */
    else if (strcmp(imsg->command, "NICK") == 0){
        if (imsg->nparam != 0) goto bad;

        ui_chan_user_list_rename_broadcast(irc.chans, imsg->nick, imsg->message);

        if (strncmp(irc.nick, imsg->nick, NICK_LEN) == 0)
            irc_nick_ack(&irc, imsg->message);
    }

    /* Names (Channel name list) */
    else if (strcmp(imsg->command, RPL_NAMREPLY) == 0){

        if (imsg->nparam != 3) goto bad;
        char *nickptr = strtok(imsg->message, " ");
        while (nickptr){
            ui_chan_user_list_add(imsg->param[2],
                    nickptr[0] == '@' ? nickptr + 1 : nickptr,
                    nickptr[0] == '@' ? IRC_USER_OP : IRC_USER_PERSON,
                    0);
            nickptr = strtok(NULL, " ");
        }
    }

    /* NOTICE */
    else if (strcmp(imsg->command, "NOTICE") == 0){
        if (imsg->nparam != 1) goto bad;
        if (strcmp(imsg->param[0], "*") == 0) {
            ui_msg_recv_broadcast(irc.chans,
                    strlen(imsg->nick) ? imsg->nick : imsg->servername,
                    "", imsg->message);
        }
        else if (strcmp(imsg->param[0], irc.nick) == 0) {
            ui_msg_recv(META_SERVER, strlen(imsg->nick) ? imsg->nick : imsg->servername,
                    "", imsg->message);
        } else {
            ui_msg_recv(imsg->param[0], strlen(imsg->nick) ? imsg->nick : imsg->servername,
                    "", imsg->message);
        }
    }

    /* MODE */
    /* TODO: no sure it is a correct way */
    else if (strcmp(imsg->command, "MODE") == 0){
        /* User modes */
        if (strlen(imsg->message) != 0){
            if (imsg->nparam != 1) goto bad;
            ui_msg_sysf_broadcast(irc.chans, SYS_MSG_NORMAL, "mode %s %s by %s",
                    imsg->param[0], imsg->message, imsg->servername);
        }
        /* Channel modes */
        else {
            int i = 0;
            GString *buf = g_string_new(NULL);
            while (i < imsg->nparam){
                g_string_append_printf(buf, "%s ", imsg->param[i++]);
            }
            ui_msg_sysf(imsg->param[0] ,SYS_MSG_NORMAL, "mode %s by %s",
                    buf->str, strlen(imsg->nick) ? imsg->nick : imsg->servername);
            g_string_free(buf, TRUE);
        }
    }
    else if (strcmp(imsg->command, RPL_CHANNELMODEIS) == 0){
        if (imsg->nparam != 3) goto bad;
        ui_msg_sysf(imsg->param[1], SYS_MSG_NORMAL, "mode %s %s by %s",
           imsg->param[1], imsg->param[2],
           strlen(imsg->nick) ? imsg->nick : imsg->servername);
    }
    else if (strcmp(imsg->command, RPL_UMODEIS) == 0){
        if (imsg->nparam != 2) goto bad;
        ui_msg_sysf(NULL, SYS_MSG_NORMAL, "mode %s %s by %s",
           imsg->param[0], imsg->param[1],
           strlen(imsg->nick) ? imsg->nick : imsg->servername);
    }

    /* WHOIS TODO */
    else if (strcmp(imsg->command, RPL_WHOISUSER) == 0
            || strcmp(imsg->command, RPL_WHOISCHANNELS) == 0
            || strcmp(imsg->command, RPL_WHOISSERVER) == 0
            || strcmp(imsg->command, RPL_WHOISIDLE) == 0
            || strcmp(imsg->command, "378") == 0
            || strcmp(imsg->command, "330") == 0
            || strcmp(imsg->command, "671") == 0
            || strcmp(imsg->command, RPL_ENDOFWHOIS) == 0){
        static GString *whois_buf = NULL;

        /* first whois message */
        if (strcmp(imsg->command, RPL_WHOISUSER) == 0){
            if (imsg->nparam != 5) goto bad;
            if (whois_buf != NULL) {
                /* giveup the previous message */
                ERR_FR("recv the second RPL_WWHOISUSER before RPL_ENDOFWHOIS");
                g_string_free(whois_buf, TRUE);
                whois_buf = NULL;
            }
            whois_buf = g_string_new(NULL);
            g_string_printf(whois_buf, "%s <%s@%s> %s\n",
                    imsg->param[1], imsg->param[2],
                    imsg->param[3], imsg->message);
        } else {
            if (whois_buf == NULL) {
                ERR_FR("whois_buf is NULL, when recv WHOIS message");
                goto bad;
            }

            if (strcmp(imsg->command, RPL_WHOISCHANNELS) == 0){
                if (imsg->nparam != 2) goto bad;
                g_string_append_printf(whois_buf, "%s is member of %s\n",
                        imsg->param[1], imsg->message);
            }
            else if (strcmp(imsg->command, RPL_WHOISSERVER) == 0){
                if (imsg->nparam != 3) goto bad;
                g_string_append_printf(whois_buf, "%s is attacht to %s at \"%s\"\n",
                        imsg->param[1], imsg->param[2], imsg->message);
            }
            else if (strcmp(imsg->command, RPL_WHOISIDLE) == 0){
                if (imsg->nparam != 4) goto bad;
                g_string_append_printf(whois_buf, "%s is idle for %s seconds since %s\n",
                        imsg->param[1], imsg->param[2], imsg->param[3]);
            }
            else if (strcmp(imsg->command, "330") == 0){
                if (imsg->nparam != 3) goto bad;
                g_string_append_printf(whois_buf, "%s %s %s\n",
                        imsg->param[1], imsg->message, imsg->param[2]);
            }
            else if (strcmp(imsg->command, "378") == 0
                    || strcmp(imsg->command, "671") == 0){
                if (imsg->nparam != 2) goto bad;
                g_string_append_printf(whois_buf, "%s %s\n",
                        imsg->param[1], imsg->message);
            }
            /* end of whois message */
            else if (strcmp(imsg->command, RPL_ENDOFWHOIS) == 0){
                g_string_append(whois_buf, imsg->message);
                ui_msg_sys(NULL, SYS_MSG_NORMAL, whois_buf->str);

                g_string_free(whois_buf, TRUE);
                whois_buf = NULL;
            }
        }
    }

    /* Other Server Mesage (receive when login) */
    else if (strcmp(imsg->command, RPL_WELCOME) == 0
            || strcmp(imsg->command, RPL_YOURHOST) == 0
            || strcmp(imsg->command, RPL_CREATED) == 0
            || strcmp(imsg->command, RPL_MOTDSTART) == 0
            || strcmp(imsg->command, RPL_MOTD) == 0
            || strcmp(imsg->command, RPL_ENDOFMOTD) == 0
            || strcmp(imsg->command, RPL_MYINFO) == 0
            || strcmp(imsg->command, RPL_BOUNCE) == 0
            || strcmp(imsg->command, RPL_LUSEROP) == 0
            || strcmp(imsg->command, RPL_LUSERUNKNOWN) == 0
            || strcmp(imsg->command, RPL_LUSERCHANNELS) == 0
            || strcmp(imsg->command, RPL_LUSERCLIENT) == 0
            || strcmp(imsg->command, RPL_LUSERME) == 0
            || strcmp(imsg->command, RPL_ADMINME) == 0
            || strcmp(imsg->command, "250") == 0
            || strcmp(imsg->command, "265") == 0
            || strcmp(imsg->command, "266") == 0
            ){
        /* regard a server message as a PRVIMSG message
         * let nick = servername
         *     msg = param[1..n] + message
         *     param[0] is you nick
         */
        int i = 1;
        GString *buf = g_string_new(NULL);
        while (i < imsg->nparam){
            g_string_append_printf(buf, "%s ", imsg->param[i++]);
        }
        buf = g_string_append(buf, imsg->message);
        ui_msg_recv(META_SERVER, imsg->servername, irc.server, buf->str);
        g_string_free(buf, TRUE);
    }

    // IGNORE
    else if (strcmp(imsg->command, RPL_ENDOFNAMES) == 0){
        // pass
    }

    /* RPL_ERROR */
    else if (imsg->command[0] == '4'
            || imsg->command[0] == '5'){
        ui_msg_sysf(NULL, SYS_MSG_ERROR, "ERROR [%s]: %s", imsg->command, imsg->message);
        ERR_FR("error message:");
        goto bad;
    }

    /* unsupported message */
    else {
        ERR_FR("unsupported message");
        goto bad;
    }

    free(imsg);
    return FALSE;   // you must return FALSE when execute normally

bad:
    ERR_FR("bad message:");
    LOG("\tnick: %s\n", imsg->nick);
    LOG("\tservername: %s\n", imsg->servername);
    LOG("\tnparam: %d\n", imsg->nparam);
    LOG("\tparam: ");
    int i = imsg->nparam;
    while (i--) LOG("%s(%d) ", imsg->param[i], i);
    LOG("\n");
    LOG("\tcommand: %s\n", imsg->command);
    LOG("\tmessage: %s\n", imsg->message);

    free(imsg);
    return FALSE;   // you must return FALSE when execute normally
}

/* this function work in listening thread */
void srain_recv(){
    irc_msg_t *imsg;
    irc_msg_type_t type;

    LOG_FR("start listening in a new thread");

    for (;;){
        imsg = calloc(1, sizeof(irc_msg_t));
        type = irc_recv(&irc, imsg);

        if (type == IRCMSG_SCKERR){
            ERR_FR("socket error, listen thread stop, connection close");
            irc_close(&irc);
            return;
        }
        if (type == IRCMSG_MSG){
            // let main loop process data
            gdk_threads_add_idle((GSourceFunc)srain_idles, imsg);
        } else {
            free(imsg);
        }
    }
}

void srain_close(){
    irc_quit_req(&irc, META_NAME_VERSION);
    irc_close(&irc);
    // TODO: close client
}

#define IS_CMD(x, y) (strncmp(x, y, strlen(y)) == 0 && \
        (x[strlen(y)] == '\0' || x[strlen(y)] == ' '))
int srain_cmd(const char *chan, char *cmd){
    if (strncmp(cmd, "/help", 5) == 0){
        static char help[] = META_CMD_HELP;

        ui_msg_sys(NULL, SYS_MSG_NORMAL, help);
        return 0;
    }
    else if (IS_CMD(cmd, "/connect")){
        char *server = strtok(cmd + strlen("/connect"), " ");
        if (server) return srain_connect(server);
    }
    else if (IS_CMD(cmd, "/login")){
        char *nick = strtok(cmd + strlen("/login"), " ");
        if (nick) return srain_login(nick);
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

    /**************************************/
    else if (IS_CMD(cmd, "/query")){
        char *target = strtok(cmd + strlen("/query"), " ");
        if (target) return srain_query(target);
    }
    else if (IS_CMD(cmd, "/unquery")){
        char *target = strtok(cmd + strlen("/unquery"), " ");
        if (target == NULL) target = (char *)chan;
            return srain_unquery(target);
    }
    else if (IS_CMD(cmd, "/join")){
        char *jchan = strtok(cmd + strlen("/join"), " ");
        if (jchan) return srain_join(jchan);
    }
    else if (IS_CMD(cmd, "/part")){
        char *pchan = strtok(cmd + strlen("/part"), " ");
        if (pchan == NULL) pchan = (char *)chan;
        return srain_part(pchan, NULL);
    }
    else if (IS_CMD(cmd, "/quit")){
        srain_close();
        return 0;
    }
    else if (IS_CMD(cmd, "/msg")){
        char *to = strtok(cmd + strlen("/msg"), " ");
        char *msg = strtok(NULL, "");
        if (to && msg) return srain_send(to, msg);
    }
    else if (IS_CMD(cmd, "/me")){
        char *msg = cmd + 4;
        if (msg){
            ui_msg_sysf(chan, SYS_MSG_ACTION, "*** %s %s ***", irc.nick, msg);
            return irc_send(&irc, chan, msg, 1);
        }
    }
    else if (IS_CMD(cmd, "/nick")){
        char *nick = strtok(cmd + strlen("/nick"), " ");
        if (nick){
            /* irc->nick will be modified when recv
             * NICK command from server */
            return irc_nick_req(&irc, nick);
        }
    }
    else if (IS_CMD(cmd, "/whois")){
        char *nick = strtok(cmd + strlen("/whois"), " ");
        if (nick == NULL) nick = (char *)chan;
        return irc_whois(&irc, nick);
    }
    else if (IS_CMD(cmd, "/invite")){
        char *nick = strtok(cmd + strlen("/invite"), " ");
        char *ichan = strtok(NULL, " ");
        if (nick){
            if (ichan == NULL) ichan = (char *)ui_chan_get_cur_name();
            ui_msg_sysf(chan, SYS_MSG_NORMAL, "You have invited %s to %s",
                    nick, ichan);
            return irc_invite(&irc, nick, ichan);
        }
    }
    else if (IS_CMD(cmd, "/kick")){
        char *nick = strtok(cmd + strlen("/kick"), " ");
        char *kchan = strtok(NULL, " ");
        char *reason = strtok(NULL, "");
        if (nick){
            if (kchan == NULL) kchan = (char *)ui_chan_get_cur_name();
            if (reason == NULL) reason = "";
            return irc_kick(&irc, nick, chan, reason);
        }
    }
    else if (IS_CMD(cmd, "/mode")){
        char *target = strtok(cmd + strlen("/mode"), " ");
        char *mode = strtok(NULL, "");
        if (target){
            if (mode == NULL) mode = "";
            return irc_mode(&irc, target, mode);
        }
    } else {
        ui_msg_sysf(chan, SYS_MSG_ERROR, "%s: unsupported command", cmd);
        return -1;
    }

    ui_msg_sysf(chan, SYS_MSG_ERROR, "missing parameter");
    return -1;
}
