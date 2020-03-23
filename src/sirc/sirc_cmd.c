/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file sirc_cmd.c
 * @brief IRC client commands
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 *
 * Originated from <https://github.com/Themaister/simple-irc-bot>.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "sirc/sirc.h"
#include "io_stream.h"
#include "sirc_cmd_builder.h"

#include "srain.h"
#include "log.h"
#include "utils.h"

int sirc_cmd_ping(SircSession *sirc, const char *data){
    g_return_val_if_fail(!str_is_empty(data), SRN_ERR);

    return sirc_cmd_raw(sirc, "PING :%s\r\n", data);
}

// sirc_cmd_pong: For answering pong requests...
int sirc_cmd_pong(SircSession *sirc, const char *data){
    g_return_val_if_fail(!str_is_empty(data), SRN_ERR);

    return sirc_cmd_raw(sirc, "PONG :%s\r\n", data);
}

int sirc_cmd_user(SircSession *sirc, const char *username, const char *hostname,
        const char *servername, const char *realname){
    g_return_val_if_fail(!str_is_empty(username), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(hostname), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(servername), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(realname), SRN_ERR);

    return sirc_cmd_raw(sirc, "USER %s %s %s :%s\r\n",
            username, hostname, servername, realname);
}

// sirc_cmd_join: For joining a chan
int sirc_cmd_join(SircSession *sirc, const char *chan, const char *passwd){
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);

    if (passwd) {
        return sirc_cmd_raw(sirc, "JOIN %s :%s\r\n", chan, passwd);
    } else {
        return sirc_cmd_raw(sirc, "JOIN %s\r\n", chan);
    }
}

// sirc_cmd_part: For leaving a chan
int sirc_cmd_part(SircSession *sirc, const char *chan, const char *reason){
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);

    if (reason) {
        return sirc_cmd_raw(sirc, "PART %s :%s\r\n", chan, reason);
    } else {
        return sirc_cmd_raw(sirc, "PART %s\r\n", chan);
    }
}

// sirc_cmd_nick: For changing your nick
int sirc_cmd_nick(SircSession *sirc, const char *nick){
    g_return_val_if_fail(!str_is_empty(nick), SRN_ERR);

    return sirc_cmd_raw(sirc, "NICK %s\r\n", nick);
}

// sirc_cmd_quit: For quitting IRC
int sirc_cmd_quit(SircSession *sirc, const char *reason){
    if (reason){
        return sirc_cmd_raw(sirc, "QUIT :%s\r\n", reason);
    } else {
        return sirc_cmd_raw(sirc, "QUIT\r\n");
    }
}


// sirc_cmd_topic: For setting or removing a topic
int sirc_cmd_topic(SircSession *sirc, const char *chan, const char *topic){
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);

    if (topic) {
        // Clear (while topic == "") or set the topic of channel
        return sirc_cmd_raw(sirc, "TOPIC %s :%s\r\n", chan, topic);
    } else {
        // Return the topic of channel
        return sirc_cmd_raw(sirc, "TOPIC %s\r\n", chan);
    }
}

// sirc_cmd_action: For executing an action (.e.g /me is hungry)
int sirc_cmd_action(SircSession *sirc, const char *target, const char *msg){
    return sirc_cmd_ctcp_req(sirc, target, "ACTION", msg);
}

// sirc_cmd_msg: For sending a chan message or a query
int sirc_cmd_msg(SircSession *sirc, const char *chan, const char *msg){
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(msg), SRN_ERR);

    while (msg) {
        SircCommandBuilder *builder = sirc_command_builder_new("PRIVMSG");
        if (!sirc_command_builder_add_middle(builder, chan)) {
            sirc_command_builder_free(builder);
            g_warn_if_reached();
            return SRN_ERR;
        }
        msg = sirc_command_builder_set_trailing(builder, msg);
        if (msg == msg) {
            sirc_command_builder_free(builder);
            g_warn_if_reached();
            return SRN_ERR;
        }

        char *cmd = sirc_command_builder_build(builder);
        SrnRet ret = sirc_cmd_raw(sirc, "%s", cmd);
        g_free(cmd);
        sirc_command_builder_free(builder);

        if (!RET_IS_OK(ret)) {
            return ret;
        }
    }

    // Entire message has been sent
    return SRN_OK;
}

int sirc_cmd_names(SircSession *sirc, const char *chan){
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);

    return sirc_cmd_raw(sirc, "NAMES %s\r\n", chan);
}

int sirc_cmd_whois(SircSession *sirc, const char *who){
    g_return_val_if_fail(!str_is_empty(who), SRN_ERR);

    return sirc_cmd_raw(sirc, "WHOIS %s\r\n", who);
}

int sirc_cmd_invite(SircSession *sirc, const char *nick, const char *chan){
    g_return_val_if_fail(!str_is_empty(nick), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);

    return sirc_cmd_raw(sirc, "INVITE %s %s\r\n", nick, chan);
}

int sirc_cmd_kick(SircSession *sirc, const char *nick, const char *chan,
        const char *reason){
    g_return_val_if_fail(!str_is_empty(nick), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(chan), SRN_ERR);

    if (reason){
        return sirc_cmd_raw(sirc, "KICK %s %s :%s\r\n", chan, nick, reason);
    } else {
        return sirc_cmd_raw(sirc, "KICK %s %s\r\n", chan, nick);
    }
}

int sirc_cmd_mode(SircSession *sirc, const char *target, const char *mode){
    g_return_val_if_fail(!str_is_empty(target), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(mode), SRN_ERR);

    return sirc_cmd_raw(sirc, "MODE %s %s\r\n", target, mode);
}

int sirc_cmd_pass(SircSession *sirc, const char *pass){
    g_return_val_if_fail(!str_is_empty(pass), SRN_ERR);

    return sirc_cmd_raw(sirc, "PASS :%s\r\n", pass);
}

int sirc_cmd_list(SircSession *sirc, const char *chan, const char *target){
    if (!str_is_empty(chan)){
        if (!str_is_empty(target)){
            return sirc_cmd_raw(sirc, "LIST %s %s\r\n", chan, target);
        } else {
            return sirc_cmd_raw(sirc, "LIST %s\r\n", chan);
        }
    } else {
            return sirc_cmd_raw(sirc, "LIST\r\n");
    }
}

int sirc_cmd_ctcp_req(SircSession *sirc, const char *target, const char *cmd,
        const char *msg){
    g_return_val_if_fail(!str_is_empty(target), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(cmd), SRN_ERR);

    /* CTCP queries are sent with PRIVMSG */
    if (msg) {
        return sirc_cmd_raw(sirc, "PRIVMSG %s :\001%s %s\001\r\n",
                target, cmd, msg);
    } else {
        return sirc_cmd_raw(sirc, "PRIVMSG %s :\001%s\001\r\n", target, cmd);
    }
}

int sirc_cmd_ctcp_rsp(SircSession *sirc, const char *target, const char *cmd,
        const char *msg){
    g_return_val_if_fail(!str_is_empty(target), SRN_ERR);
    g_return_val_if_fail(!str_is_empty(cmd), SRN_ERR);

    /* CTCP queries are sent with NOTICE */
    if (msg) {
        return sirc_cmd_raw(sirc, "NOTICE %s :\001%s %s\001\r\n",
                target, cmd, msg);
    } else {
        return sirc_cmd_raw(sirc, "NOTICE %s :\001%s\001\r\n", target, cmd);
    }
}

int sirc_cmd_cap_ls(SircSession *sirc, const char *version){
    if (version){
        return sirc_cmd_raw(sirc, "CAP LS %s\r\n", version);
    } else {
        return sirc_cmd_raw(sirc, "CAP LS\r\n");
    }
}

int sirc_cmd_cap_list(SircSession *sirc){
    return sirc_cmd_raw(sirc, "CAP LIST\r\n");
}

int sirc_cmd_cap_req(SircSession *sirc, const char *caps){
    g_return_val_if_fail(caps, SRN_ERR);

    return sirc_cmd_raw(sirc, "CAP REQ :%s\r\n", caps);
}

int sirc_cmd_cap_end(SircSession *sirc){
    return sirc_cmd_raw(sirc, "CAP END\r\n");
}

int sirc_cmd_authenticate(SircSession *sirc, const char *msg){
    g_return_val_if_fail(msg, SRN_ERR);

    return sirc_cmd_raw(sirc, "AUTHENTICATE %s\r\n", msg);
}

int sirc_cmd_away(SircSession *sirc, const char *msg){
    if (msg) {
        // Set an AWAY message
        return sirc_cmd_raw(sirc, "AWAY %s\r\n", msg);
    } else {
        // Remove the AWAY message
        return sirc_cmd_raw(sirc, "AWAY\r\n");
    }
}

int sirc_cmd_raw(SircSession *sirc, const char *fmt, ...){
    char buf[SIRC_BUF_LEN];
    int len = 0;
    int msgid = sirc_get_msgid(sirc);
    va_list args;
    GIOStream *stream;

    g_return_val_if_fail(sirc, SRN_ERR);
    g_return_val_if_fail(fmt, SRN_ERR);
    stream = sirc_get_stream(sirc);
    g_return_val_if_fail(G_IS_IO_STREAM(stream), SRN_ERR);

    if (strlen(fmt) != 0){
        va_start(args, fmt);
        len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
    }
    DBG_FR("[#%d] Send raw: %s", msgid, buf);

    if (len > 512){
        WARN_FR("Raw command too long");
        len = 512;
    }

    // TODO send it totally
    msgid++;
    sirc_set_msgid(sirc, msgid);
    return (io_stream_write(stream, buf, len) < 0) ?
        SRN_ERR : SRN_OK;
}
