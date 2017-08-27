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

#include <string.h>

#include "server.h"

#include "meta.h"
#include "decorator.h"
#include "filter.h"
#include "i18n.h"
#include "command.h"
#include "log.h"
#include "prefs.h"

#include "sirc/sirc.h"

extern SuiEvents ui_events;
extern CommandBind cmd_binds[];

static void append_image(Message *msg);
static void add_message(Chat *chat, Message *msg);
static bool whether_merge_last_message(Chat *chat, Message *msg);

Chat *chat_new(Server *srv, const char *name){
    bool ischan;
    SrnRet ret;
    Chat *chat;
    SuiSessionFlag flag;

    g_return_val_if_fail(name, NULL);

    ischan = sirc_is_chan(name);
    chat = g_malloc0(sizeof(Chat));

    chat->joined = FALSE;
    chat->srv = srv;
    chat->user = user_ref(srv->user);
    // chat->user_list = NULL; // via g_malloc0()
    // chat->msg_list = NULL; // via g_malloc0()
    // chat->last_msg = NULL; // via g_malloc0()
    g_strlcpy(chat->name, name, sizeof(chat->name));

    /* sui */
    chat->ui_prefs = sui_prefs_new();
    ret = prefs_read_sui_prefs(chat->ui_prefs, chat->srv->prefs->name, chat->name);
    if (!RET_IS_OK(ret)){
        ERR_FR("Read sui prefs failed: %s", RET_MSG(ret));
    }

    flag = ischan ? SUI_SESSION_CHANNEL : SUI_SESSION_DIALOG;
    if (strcmp(META_SERVER, name) == 0){
        flag = SUI_SESSION_SERVER;
    }

    chat->ui = sui_new_session(&ui_events, chat->ui_prefs, flag);

    if (!chat->ui){
        goto cleanup;
    }

    sui_set_ctx(chat->ui, chat);
    sui_start_session(chat->ui, name, srv->prefs->name);

    /* For a dialog, its user_list must have yourself and the dialogue target */
    if (flag == SUI_SESSION_DIALOG){
        chat_add_user(chat, srv->user->nick, USER_CHIGUA);
        chat_add_user(chat, name, USER_CHIGUA);
    }

    sui_add_completion(chat->ui, chat->name);
    for (int i = 0; cmd_binds[i].name != NULL; i++){
        sui_add_completion(chat->ui, cmd_binds[i].name);
    }

    /* Require chat->ui */

    /* You will add 2 users with same nickname if you chat with yourself
    if (flag == SUI_SESSION_DIALOG){
        g_return_val_if_fail(chat->user, NULL);
        chat_add_user(chat, chat->user->nick, USER_CHIGUA);
        chat_add_user(chat, chat->name, USER_CHIGUA);
    }
    */

    return chat;

cleanup:
    if (chat->ui) {
        sui_free_session(chat->ui);
    }
    if (chat->ui_prefs) {
        g_free(chat->ui_prefs);
    }
    if (chat){
        g_free(chat);
    }

    return NULL;
}

void chat_free(Chat *chat){
    user_free(chat->user);

    /* Free extra list */
    if (chat->relaybot_list){
        relay_decroator_free_list(chat);
    }

    if (chat->ignore_nick_list){
        nick_filter_free_list(chat);
    }

    if (chat->ignore_regex_list){
        regex_filter_free_list(chat);
    }

    /* Free user list */
    if (chat->user_list){
        GSList *lst = chat->user_list;
        while (lst){
            if (lst->data){
                user_free((User *)lst->data);
                lst->data = NULL;
            }
            lst = g_slist_next(lst);
        }
        g_slist_free(chat->user_list);
        chat->user_list = NULL;
    }

    if (chat->ui){
        sui_free_session(chat->ui);
        chat->ui = NULL;
    }

    if (chat->ui_prefs){
        sui_prefs_free(chat->ui_prefs);
        chat->ui_prefs = NULL;
    }

    g_free(chat);
}

int chat_add_user(Chat *chat, const char *nick, UserType type){
    int ret;
    User *user;

    user = user_new(chat, nick, NULL, NULL, type);

    ret = chat_add_user_full(chat, user);
    if (sirc_nick_cmp(chat->user->nick, nick)){
        user_set_me(user, TRUE);
    }

    user_free(user);

    return ret;
}

int chat_add_user_full(Chat *chat, User *user){
    GSList *lst;
    User *user2;

    lst = chat->user_list;
    while (lst){
        user2 = lst->data;
        if (sirc_nick_cmp(user2->nick, user->nick)){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    g_return_val_if_fail(user, SRN_ERR);

    chat->user_list = g_slist_append(chat->user_list, user_ref(user));
    sui_add_user(chat->ui, user->nick, user->type);

    return SRN_OK;
}

int chat_rm_user(Chat *chat, const char *nick){
    GSList *lst;
    User *user;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (sirc_nick_cmp(user->nick, nick)){
            user_free(user);
            sui_rm_user(chat->ui, user->nick);
            chat->user_list = g_slist_delete_link(chat->user_list, lst);

            return SRN_OK;
        }
        lst = g_slist_next(lst);
    }
    return SRN_ERR;
}


User* chat_get_user(Chat *chat, const char *nick){
    User *user;
    GSList *lst;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (sirc_nick_cmp(user->nick, nick)){
            return user;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}


void chat_add_sent_message(Chat *chat, const char *content){
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat->user;
    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_CHAT_LOG;
    msg = message_new(chat, user, content, MESSAGE_SENT);

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        /* Ignore this message */
        goto cleanup;
    }

    if (whether_merge_last_message(chat, msg)){
        msg->ui = chat->last_msg->ui;
        sui_message_append_message(chat->ui, msg->ui, msg->dcontent);
    } else {
        msg->ui = sui_add_sent_msg(chat->ui, msg->dcontent);
    }

    if (!msg->ui){
        goto cleanup;
    }

    sui_message_set_time(msg->ui, msg->time);

    append_image(msg);

    add_message(chat, msg);

    return;

cleanup:
    message_free(msg);
}

void chat_add_recv_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    dflag = DECORATOR_PANGO_MARKUP |DECORATOR_RELAY | DECORATOR_MIRC_STRIP | DECORATOR_MENTION;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    msg = message_new(chat, user, content, MESSAGE_RECV);

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    if (whether_merge_last_message(chat, msg)){
        msg->ui = chat->last_msg->ui;
        sui_message_append_message(chat->ui, msg->ui, msg->dcontent);
    } else {
        msg->ui = sui_add_recv_msg(chat->ui, msg->dname, msg->role, msg->dcontent);
    }

    if (!msg->ui){
        goto cleanup;
    }

    if (msg->mentioned){
        sui_message_mentioned(msg->ui);
        sui_message_notify(msg->ui);
    }
    else if (chat != chat->srv->chat && !sirc_is_chan(chat->name)){
        sui_message_notify(msg->ui);
    }

    sui_message_set_time(msg->ui, msg->time);

    append_image(msg);

    add_message(chat, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_notice_message(Chat *chat, const char *origin, const char *content){
    chat_add_recv_message(chat, origin, content);
}

void chat_add_action_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    FilterFlag fflag;
    DecoratorFlag dflag;

    user = chat_get_user(chat, origin);

    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_CHAT_LOG;

    msg = message_new(chat, user, content, MESSAGE_ACTION);

    if (user->me){
    } else {
        fflag |= FILTER_NICK | FILTER_REGEX;
        dflag |= DECORATOR_RELAY | DECORATOR_MIRC_STRIP | DECORATOR_MENTION;
    }

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    {
        // TODO: "<b>" no used?
        char *action_msg = g_strdup_printf(_("*** <b>%s</b> %s ***"),
                msg->dname, msg->dcontent);

        msg->ui = sui_add_sys_msg(chat->ui, action_msg, SYS_MSG_ACTION);
        g_free(action_msg);
    }


    if (!msg->ui){
        goto cleanup;
    }

    if (msg->mentioned){
        sui_message_mentioned(msg->ui);
        sui_message_notify(msg->ui);
    }

    append_image(msg);

    add_message(chat, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_misc_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = message_new(chat, user, content, MESSAGE_MISC);

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    msg->ui = sui_add_sys_msg(chat->ui, msg->dcontent, SYS_MSG_NORMAL);
    if (!msg->ui){
        goto cleanup;
    }

    add_message(chat, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_misc_message_fmt(Chat *chat, const char *origin, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    chat_add_misc_message(chat, origin, content);

    g_free(content);
}

void chat_add_error_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = message_new(chat, user, content, MESSAGE_ERROR);

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    msg->ui = sui_add_sys_msg(chat->ui, msg->dcontent, SYS_MSG_ERROR);

    if (!msg->ui){
        goto cleanup;
    }

    sui_message_notify(msg->ui);

    add_message(chat, msg);

    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_error_message_fmt(Chat *chat, const char *origin, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    chat_add_error_message(chat, origin, content);

    g_free(content);
}

void chat_set_topic(Chat *chat, const char *origin, const char *topic){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_MIRC_STRIP | DECORATOR_PANGO_MARKUP;
    msg = message_new(chat, user, topic, MESSAGE_UNKNOWN);

    if (decorate_message(msg, dflag, NULL) == SRN_OK){
        sui_set_topic(chat->ui, msg->dcontent);
    }

    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_set_topic_setter(Chat *chat, const char *setter){
    sui_set_topic_setter(chat->ui, setter);
}

static void add_message(Chat *chat, Message *msg){
    chat->msg_list = g_list_append(chat->msg_list, msg);
    chat->last_msg = msg;
}

static bool whether_merge_last_message(Chat *chat, Message *msg){
    Message *last_msg;

    last_msg = chat->last_msg;

    return (last_msg
            && msg->time - last_msg->time < MESSAGE_MERGE_INTERVAL
            && (!last_msg->mentioned && !msg->mentioned)
            && last_msg->type == msg->type
            && sirc_nick_cmp(last_msg->user->nick, msg->user->nick)
            && sirc_nick_cmp(last_msg->dname, msg->dname));
}

// TODO
static void append_image(Message *msg){
    GSList *url;

    url = msg->urls;

    while (url){
        if (g_str_has_prefix(url->data, "http") // Both "http" and "https"
                && (g_str_has_suffix(url->data, "png")
                    || g_str_has_suffix(url->data, "jpg")
                    || g_str_has_suffix(url->data, "jpeg")
                    || g_str_has_suffix(url->data, "bmp")
                    || g_str_has_suffix(url->data, "gif")
                    )){
            sui_message_append_image(msg->ui, url->data);
        }
        url = g_slist_next(url);
    }
}
