/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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

#include <glib.h>

#include "server.h"

#include "decorator.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "log.h"
#include "i18n.h"
#include "utils.h"

static char* relay(Message *msg, int index, const char *frag);

Decorator relay_decroator = {
    .name = "relay",
    .func = relay,
};

int relay_decroator_add_nick(Chat *chat, const char *nick){
    GSList *lst;

    lst = chat->relaybot_list;

    while(lst){
        if (sirc_nick_cmp(lst->data, nick)){
            chat_add_error_message_fmt(chat->srv->cur_chat, chat->user->nick,
                    _("\"%s\" already exists in %s 's relaybot list"),
                    nick, chat->name);
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    chat->relaybot_list = g_slist_append(chat->relaybot_list, g_strdup(nick));

    chat_add_misc_message_fmt(chat->srv->cur_chat, chat->user->nick,
            _("\"%s\" has added to %s 's relaybot list"), nick, chat->name);

    return SRN_OK;
}

int relay_decroator_rm_nick(Chat *chat, const char *nick){
    GSList *lst;

    lst = chat->relaybot_list;

    while(lst){
        if (lst->data){
            if (sirc_nick_cmp(lst->data, nick)){
                g_free(lst->data);
                chat->relaybot_list = g_slist_delete_link(chat->relaybot_list, lst);

                chat_add_misc_message_fmt(chat->srv->cur_chat, chat->user->nick,
                        _("\"%s\" is removed from %s 's relaybot list"),
                        nick, chat->name);

                return SRN_OK;
            }
        }
        lst = g_slist_next(lst);
    }

    chat_add_error_message_fmt(chat->srv->cur_chat, chat->user->nick,
            _("\"%s\" not found in %s 's relaybot list"),
            nick, chat->name);

    return SRN_ERR;
}

void relay_decroator_free_list(Chat *chat){
    g_slist_free_full(chat->relaybot_list, g_free);
    chat->relaybot_list = NULL;
}

static char* relay(Message *msg, int index, const char *frag){
    char *dnick;
    char *dcontent = NULL;
    GSList *lst;
    GError *err;
    GRegex *regex;
    GMatchInfo *match_info;

    if (index != 0) {
        DBG_FR("Only accept 0 while index = %d", index);
        return NULL;
    }

    /* ref: https://github.com/tuna/scripts/blob/master/weechat_bot2human.py#L46 */
    err = NULL;
    regex = g_regex_new("(\x03[0-9,]+)?\\[(?<nick>[^:]+?)\\]\x0f? (?<text>.*)", 0, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed: %s", err->message);
        return NULL;
    }

    lst = msg->chat->relaybot_list;
    while (lst){
        if (sirc_nick_cmp(msg->user->nick, lst->data)){
            DBG_FR("Brige bot '%s' found", (char *)lst->data);
            g_regex_match(regex, frag, 0, &match_info);

            if (g_match_info_matches(match_info)){
                dnick = g_match_info_fetch_named(match_info, "nick");
                dcontent = g_match_info_fetch_named(match_info, "text");

                str_assign(&msg->dname, dnick);
                str_assign(&msg->role, msg->user->nick);

                LOG_FR("Relay message matched, nick: %s, content: %s", dnick, dcontent);

                g_match_info_free(match_info);
                goto FIN;
            }
            g_match_info_free(match_info);
        }
        lst = g_slist_next(lst);
    }

    lst = msg->chat->srv->chat->relaybot_list;
    while (lst){
        if (sirc_nick_cmp(msg->user->nick, lst->data)){
            DBG_FR("Brige bot '%s' found", (char *)lst->data);
            g_regex_match(regex, frag, 0, &match_info);

            if (g_match_info_matches(match_info)){
                dnick = g_match_info_fetch_named(match_info, "nick");
                dcontent = g_match_info_fetch_named(match_info, "text");

                str_assign(&msg->dname, dnick);
                str_assign(&msg->role, msg->user->nick);

                LOG_FR("Relay message matched, nick: %s, content: %s", dnick, dcontent);

                g_match_info_free(match_info);
                goto FIN;
            }
            g_match_info_free(match_info);
        }
        lst = g_slist_next(lst);
    }

FIN:
    g_regex_unref(regex);

    return dcontent;
}
