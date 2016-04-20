#define __LOG_ON

#include <glib.h>
#include <string.h>
#include <strings.h>
#include "irc.h"
#include "irc_magic.h"
#include "meta.h"
#include "srain_msg.h"
#include "filter.h"
#include "log.h"
#include "server.h"
#include "server_intf.h"

/* GSourceFunc */
gboolean server_msg_dispatch(IRCMsg *imsg){
    /* where is this message comes from */
    IRCServer *srv = imsg->server;

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
            server_intf_ui_sys_msg(srv, dest, "ACTION recv", SYS_MSG_ACTION);
            // ui_msg_sysf(srv, dest, SYS_MSG_ACTION, "*** %s %s ***",
                    // imsg->nick, imsg->message);
        } else {
            server_intf_ui_recv_msg(srv, dest, imsg->nick,
                    imsg->servername, imsg->message);
        }
    }

    /* Topic */
    else if (strcmp(imsg->command, "TOPIC") == 0){
        if (imsg->nparam != 1) goto bad;
        server_intf_ui_set_topic(srv, imsg->param[0], imsg->message);
    }
    else if (strcmp(imsg->command, RPL_TOPIC) == 0){
        if (imsg->nparam != 2) goto bad;
        server_intf_ui_set_topic(srv, imsg->param[1], imsg->message);
    }

    /* JOIN & PART & QUIT
     *
     * sys message "xxx has join/leave #yyy" are sent
     * by ui_chan_user_list_{add,rm}()
     */
    else if (strcmp(imsg->command, "JOIN") == 0){
        if (imsg->nparam != 1) goto bad;
        if (strncasecmp(imsg->nick, srv->irc.nick, NICK_LEN) == 0){
            server_intf_ui_join(srv, imsg->param[0]);
            // irc_join_ack(&irc, imsg->param[0]);
        }
        server_intf_ui_user_join(srv, imsg->param[0], imsg->nick,
                IRC_USER_PERSON, 1);
    }
    else if (strcmp(imsg->command, "PART") == 0){
        if (imsg->nparam != 1) goto bad;
        server_intf_ui_user_part(srv, imsg->param[0], imsg->nick, imsg->message);
        if (strncasecmp(imsg->nick, srv->irc.nick, NICK_LEN) == 0){
            // irc_part_ack(&irc, imsg->param[0]);
            server_intf_ui_part(srv, imsg->param[0]);
        }
    }
    else if (strcmp(imsg->command, "QUIT") == 0){
        if (imsg->nparam != 0) goto bad;
        // ui_chan_user_list_rm_broadcast(srv, imsg->nick, imsg->message);
        // TODO
    }

    /* INVITE & KICK */
    else if (strcmp(imsg->command, "INVITE") == 0){
        if (imsg->nparam != 1) goto bad;
        // ui_msg_sysf(srv, imsg->nick, SYS_MSG_NORMAL, "%s invites you into %s",
                // imsg->nick, imsg->message);
        server_intf_ui_sys_msg(srv, imsg->nick, "invite recv", SYS_MSG_NORMAL);
        goto bad;
    }
    else if (strcmp(imsg->command, "KICK") == 0){
        if (imsg->nparam != 2) goto bad;
        // ui_msg_sysf(srv, imsg->param[0], SYS_MSG_ERROR, "%s are kicked from %s by %s",
                // imsg->param[1], imsg->param[0], imsg->nick);
        server_intf_ui_sys_msg(srv, imsg->nick, "kick recv", SYS_MSG_NORMAL);
    }

    /* NICK (someone change his name) */
    else if (strcmp(imsg->command, "NICK") == 0){
        if (imsg->nparam != 0) goto bad;

        // ui_chan_user_list_rename_broadcast(srv, imsg->nick, imsg->message);
        // TODO

        if (strncmp(srv->irc.nick, imsg->nick, NICK_LEN) == 0)
            irc_nick_ack(&(srv->irc), imsg->message);
    }

    /* Names (Channel name list) */
    else if (strcmp(imsg->command, RPL_NAMREPLY) == 0){

        if (imsg->nparam != 3) goto bad;
        char *nickptr = strtok(imsg->message, " ");
        while (nickptr){
            server_intf_ui_user_join(srv, imsg->param[2],
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
            // ui_msg_recv_broadcast(srv,
                    // strlen(imsg->nick) ? imsg->nick : imsg->servername,
                    // "", imsg->message);
                    // TODO
        }
        else if (strncasecmp(imsg->param[0], srv->irc.nick, NICK_LEN) == 0) {
            server_intf_ui_recv_msg(srv, META_SERVER,
                    strlen(imsg->nick) ? imsg->nick : imsg->servername,
                    "", imsg->message);
        } else {
            server_intf_ui_recv_msg(srv, imsg->param[0],
                    strlen(imsg->nick) ? imsg->nick : imsg->servername,
                    "", imsg->message);
        }
    }

    /* MODE */
    /* TODO: no sure it is a correct way */
    else if (strcmp(imsg->command, "MODE") == 0){
        /* User modes */
        if (strlen(imsg->message) != 0){
            if (imsg->nparam != 1) goto bad;
            // ui_msg_sysf_broadcast(srv, SYS_MSG_NORMAL, "mode %s %s by %s",
                    // imsg->param[0], imsg->message, imsg->servername);
                    // TODO
        }
        /* Channel modes */
        else {
            int i = 0;
            GString *buf = g_string_new(NULL);
            while (i < imsg->nparam){
                g_string_append_printf(buf, "%s ", imsg->param[i++]);
            }
            // ui_msg_sysf(srv, imsg->param[0] ,SYS_MSG_NORMAL, "mode %s by %s",
                    // buf->str, strlen(imsg->nick) ? imsg->nick : imsg->servername);
            server_intf_ui_sys_msg(srv, imsg->param[0], "MODE recv", SYS_MSG_ACTION);
            g_string_free(buf, TRUE);
        }
    }
    else if (strcmp(imsg->command, RPL_CHANNELMODEIS) == 0){
        if (imsg->nparam != 3) goto bad;
        // ui_msg_sysf(srv, imsg->param[1], SYS_MSG_NORMAL, "mode %s %s by %s",
           // imsg->param[1], imsg->param[2],
           // strlen(imsg->nick) ? imsg->nick : imsg->servername);
        server_intf_ui_sys_msg(srv, imsg->param[1], "RPL_CHANNELMODEIS recv", SYS_MSG_ACTION);
    }
    else if (strcmp(imsg->command, RPL_UMODEIS) == 0){
        if (imsg->nparam != 2) goto bad;
        // ui_msg_sysf(srv, imsg->nick, SYS_MSG_NORMAL, "mode %s %s by %s",
                // imsg->param[0], imsg->param[1],
                // strlen(imsg->nick) ? imsg->nick : imsg->servername);
        server_intf_ui_sys_msg(srv, imsg->nick, "RPL_UMODEIS recv", SYS_MSG_ACTION);
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
                // ui_msg_sys(srv, NULL, SYS_MSG_NORMAL, whois_buf->str);
                server_intf_ui_sys_msg(srv, NULL, whois_buf->str, SYS_MSG_NORMAL);

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
        /* regard a srv message as a PRVIMSG message
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
        server_intf_ui_recv_msg(srv, META_SERVER, imsg->servername, srv->irc.server, buf->str);
        g_string_free(buf, TRUE);
    }

    // IGNORE
    else if (strcmp(imsg->command, RPL_ENDOFNAMES) == 0){
        // pass
    }

    /* RPL_ERROR */
    else if (imsg->command[0] == '4'
            || imsg->command[0] == '5'){
        // ui_msg_sysf(NULL, SYS_MSG_ERROR, "ERROR [%s]: %s", imsg->command, imsg->message);
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
