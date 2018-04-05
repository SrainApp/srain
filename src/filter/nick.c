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
 * @file nick.c
 * @brief Nick filter
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-04-19
 */

#include <string.h>
#include <glib.h>

#include "sirc/sirc.h"

#include "core/core.h"

#include "filter.h"

#include "srain.h"
#include "log.h"
#include "i18n.h"

static bool nick(const SrnMessage *msg, const char *content);

Filter nick_filter = {
    .name = "nick",
    .func = nick,
};

int nick_filter_add_nick(SrnChat *chat, const char *nick){
    GSList *lst;

    lst = chat->ignore_nick_list;

    while(lst){
        if (sirc_target_equal(lst->data, nick)){
            srn_chat_add_error_message_fmt(chat->srv->cur_chat, chat->user,
                    _("\"%1$s\" already exists in %2$s 's ignore list"),
                    nick, chat->name);
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    chat->ignore_nick_list = g_slist_append(chat->ignore_nick_list, g_strdup(nick));

    srn_chat_add_misc_message_fmt(chat->srv->cur_chat, chat->user,
            _("\"%1$s\" has added to %2$s 's ignore list"), nick, chat->name);

    return SRN_OK;
}

int nick_filter_rm_nick(SrnChat *chat, const char *nick){
    GSList *lst;

    lst = chat->ignore_nick_list;

    while(lst){
        if (lst->data){
            if (sirc_target_equal(lst->data, nick)){
                g_free(lst->data);
                chat->ignore_nick_list = g_slist_delete_link(chat->ignore_nick_list, lst);

                srn_chat_add_misc_message_fmt(chat->srv->cur_chat, chat->user,
                        _("\"%1$s\" is removed from %2$s 's ignore list"),
                        nick, chat->name);

                return SRN_OK;
            }
        }
        lst = g_slist_next(lst);
    }

    srn_chat_add_error_message_fmt(chat->srv->cur_chat, chat->user,
            _("\"%1$s\" not found in %2$s 's ignore list"),
            nick, chat->name);

    return SRN_ERR;
}

void nick_filter_free_list(SrnChat *chat){
    g_slist_free_full(chat->ignore_nick_list, g_free);
    chat->ignore_nick_list = NULL;
}

static bool nick(const SrnMessage *msg, const char *content){
    GSList *lst;

    g_return_val_if_fail(msg->chat, TRUE);
    g_return_val_if_fail(srn_server_is_valid(msg->chat->srv), TRUE);
    g_return_val_if_fail(msg->chat->srv->chat, TRUE);

    lst = msg->chat->ignore_nick_list;
    while (lst){
        if (sirc_target_equal(lst->data, msg->dname)){
            return FALSE;
        }
        lst = g_slist_next(lst);
    }

    lst = msg->chat->srv->chat->ignore_nick_list;
    while (lst){
        if (sirc_target_equal(lst->data, msg->dname)){
            return FALSE;
        }
        lst = g_slist_next(lst);
    }

    return TRUE;
}
