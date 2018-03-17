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

#include <string.h>

#include "core/core.h"

#include "meta.h"
#include "decorator.h"
#include "filter.h"
#include "i18n.h"
#include "command.h"
#include "log.h"
#include "utils.h"
#include "config/config.h"

#include "sirc/sirc.h"

extern SrnCommandBind cmd_binds[];

static void append_image(SrnMessage *msg);
static void add_message(SrnChat *chat, SrnMessage *msg);
static bool whether_merge_last_message(SrnChat *chat, SrnMessage *msg);

SrnChat *srn_chat_new(SrnServer *srv, const char *name, SrnChatConfig *cfg){
    SrnChat *chat;
    SuiBufferEvents *events;

    chat = g_malloc0(sizeof(SrnChat));

    str_assign(&chat->name, name);
    chat->srv = srv;
    chat->cfg = cfg;
    chat->joined = FALSE;
    // FIXME:
    // chat->user = srn_user_ref(srv->user);
    // chat->user_list = NULL; // via g_malloc0()
    // chat->msg_list = NULL; // via g_malloc0()
    // chat->last_msg = NULL; // via g_malloc0()

    // FIXME: ugly...
    events = &srn_application_get_default()->ui_events;
    if (strcmp(META_SERVER, chat->name) == 0){
        // Server
        chat->ui = sui_new_server_buffer(srv->cfg->name, events, cfg->ui);
    } else if (sirc_is_chan(chat->name)){
        // Channel
        chat->ui = sui_new_channel_buffer(srv->chat->ui, name, events, cfg->ui);
    } else {
        // Private, its user_list must have yourself and the dialogue target
        chat->ui = sui_new_private_buffer(srv->chat->ui, name, events, cfg->ui);
        srn_chat_add_user(chat, chat->srv->user->nick, SRN_USER_CHIGUA);
        srn_chat_add_user(chat, chat->name, SRN_USER_CHIGUA);
    }
    if (!chat->ui){
        goto cleanup;
    }
    sui_buffer_set_ctx(chat->ui, chat);

    sui_add_completion(chat->ui, chat->name);
    for (int i = 0; cmd_binds[i].name != NULL; i++){
        sui_add_completion(chat->ui, cmd_binds[i].name);
    }

    return chat;

cleanup:
    if (chat->ui) {
        sui_free_buffer(chat->ui);
    }
    if (chat->cfg) {
        srn_chat_config_free(chat->cfg);
    }
    if (chat){
        g_free(chat);
    }

    return NULL;
}

void srn_chat_free(SrnChat *chat){
    srn_user_free(chat->user);

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
                srn_user_free((SrnUser *)lst->data);
                lst->data = NULL;
            }
            lst = g_slist_next(lst);
        }
        g_slist_free(chat->user_list);
        chat->user_list = NULL;
    }

    if (chat->ui){
        sui_free_buffer(chat->ui);
        chat->ui = NULL;
    }

    g_free(chat);
}

int srn_chat_add_user(SrnChat *chat, const char *nick, UserType type){
    int ret;
    SrnUser *user;

    user = srn_user_new(chat, nick, NULL, NULL, type);

    ret = srn_chat_add_user_full(chat, user);
    if (sirc_nick_cmp(chat->user->nick, nick)){
        srn_user_set_me(user, TRUE);
    }

    srn_user_free(user);

    return ret;
}

int srn_chat_add_user_full(SrnChat *chat, SrnUser *user){
    GSList *lst;
    SrnUser *user2;

    lst = chat->user_list;
    while (lst){
        user2 = lst->data;
        if (sirc_nick_cmp(user2->nick, user->nick)){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    g_return_val_if_fail(user, SRN_ERR);

    chat->user_list = g_slist_append(chat->user_list, srn_user_ref(user));
    sui_add_user(chat->ui, user->nick, user->type);

    return SRN_OK;
}

int srn_chat_rm_user(SrnChat *chat, const char *nick){
    GSList *lst;
    SrnUser *user;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (sirc_nick_cmp(user->nick, nick)){
            sui_rm_user(chat->ui, user->nick);
            chat->user_list = g_slist_delete_link(chat->user_list, lst);
            srn_user_free(user);

            return SRN_OK;
        }
        lst = g_slist_next(lst);
    }
    return SRN_ERR;
}


SrnUser* srn_chat_get_user(SrnChat *chat, const char *nick){
    SrnUser *user;
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


void srn_chat_add_sent_message(SrnChat *chat, const char *content){
    SrnUser *user;
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat->user;
    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_CHAT_LOG;
    msg = srn_message_new(chat, user, content, SRN_MESSAGE_SENT);

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
    srn_message_free(msg);
}

void srn_chat_add_recv_message(SrnChat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    SrnUser *user;
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    dflag = DECORATOR_PANGO_MARKUP | DECORATOR_RELAY | DECORATOR_MENTION;
    if (chat->cfg->render_mirc_color) {
        dflag |= DECORATOR_MIRC_COLORIZE;
    } else {
        dflag |= DECORATOR_MIRC_STRIP;
    }
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;

    user = srn_chat_get_user(chat, origin);
    if (!user){
        user = srn_user_new(chat, origin, NULL, NULL, SRN_USER_CHIGUA);
        invalid_user = TRUE;
    }

    msg = srn_message_new(chat, user, content, SRN_MESSAGE_RECV);

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
        srn_user_free(user);
    }
    srn_message_free(msg);
}

void srn_chat_add_notice_message(SrnChat *chat, const char *origin, const char *content){
    srn_chat_add_recv_message(chat, origin, content);
}

void srn_chat_add_action_message(SrnChat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    SrnUser *user;
    SrnMessage *msg;
    FilterFlag fflag;
    DecoratorFlag dflag;

    user = srn_chat_get_user(chat, origin);

    if (!user){
        user = srn_user_new(chat, origin, NULL, NULL, SRN_USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    if (chat->cfg->render_mirc_color) {
        dflag |= DECORATOR_MIRC_COLORIZE;
    } else {
        dflag |= DECORATOR_MIRC_STRIP;
    }
    fflag = FILTER_CHAT_LOG;

    msg = srn_message_new(chat, user, content, SRN_MESSAGE_ACTION);

    if (user->me){
    } else {
        fflag |= FILTER_NICK | FILTER_REGEX;
        dflag |= DECORATOR_RELAY | DECORATOR_MENTION;
    }

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    {
        // TODO: "<b>" no used?
        char *action_msg = g_strdup_printf(_("*** <b>%1$s</b> %2$s ***"),
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
        srn_user_free(user);
    }
    srn_message_free(msg);
}

void srn_chat_add_misc_message(SrnChat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    SrnUser *user;
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = srn_chat_get_user(chat, origin);
    if (!user){
        user = srn_user_new(chat, origin, NULL, NULL, SRN_USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = srn_message_new(chat, user, content, SRN_MESSAGE_MISC);

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
        srn_user_free(user);
    }
    srn_message_free(msg);
}

void srn_chat_add_misc_message_fmt(SrnChat *chat, const char *origin, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    srn_chat_add_misc_message(chat, origin, content);

    g_free(content);
}

void srn_chat_add_error_message(SrnChat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    SrnUser *user;
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = srn_chat_get_user(chat, origin);
    if (!user){
        user = srn_user_new(chat, origin, NULL, NULL, SRN_USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = srn_message_new(chat, user, content, SRN_MESSAGE_ERROR);

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
        srn_user_free(user);
    }
    srn_message_free(msg);
}

void srn_chat_add_error_message_fmt(SrnChat *chat, const char *origin, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    srn_chat_add_error_message(chat, origin, content);

    g_free(content);
}

void srn_chat_set_topic(SrnChat *chat, const char *origin, const char *topic){
    bool invalid_user = FALSE;
    SrnUser *user;
    SrnMessage *msg;
    DecoratorFlag dflag;

    user = srn_chat_get_user(chat, origin);
    if (!user){
        user = srn_user_new(chat, origin, NULL, NULL, SRN_USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    if (chat->cfg->render_mirc_color) {
        dflag |= DECORATOR_MIRC_COLORIZE;
    } else {
        dflag |= DECORATOR_MIRC_STRIP;
    }

    msg = srn_message_new(chat, user, topic, SRN_MESSAGE_UNKNOWN);

    if (decorate_message(msg, dflag, NULL) == SRN_OK){
        sui_set_topic(chat->ui, msg->dcontent);
    }

    if (invalid_user){
        srn_user_free(user);
    }
    srn_message_free(msg);
}

void srn_chat_set_topic_setter(SrnChat *chat, const char *setter){
    sui_set_topic_setter(chat->ui, setter);
}

static void add_message(SrnChat *chat, SrnMessage *msg){
    chat->msg_list = g_list_append(chat->msg_list, msg);
    chat->last_msg = msg;
}

static bool whether_merge_last_message(SrnChat *chat, SrnMessage *msg){
    SrnMessage *last_msg;

    last_msg = chat->last_msg;

    return (last_msg
            && msg->time - last_msg->time < SRN_MESSAGE_MERGE_INTERVAL
            && (!last_msg->mentioned && !msg->mentioned)
            && last_msg->type == msg->type
            && sirc_nick_cmp(last_msg->user->nick, msg->user->nick)
            && sirc_nick_cmp(last_msg->dname, msg->dname));
}

// TODO
static void append_image(SrnMessage *msg){
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
