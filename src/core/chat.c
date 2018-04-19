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
static void add_message(SrnChat *self, SrnMessage *msg);
static bool whether_merge_last_message(SrnChat *self, SrnMessage *msg);

SrnChat* srn_chat_new(SrnServer *srv, const char *name, SrnChatType type,
        SrnChatConfig *cfg){
    SrnChat *self;
    SuiBufferEvents *events;

    self = g_malloc0(sizeof(SrnChat));

    str_assign(&self->name, name);
    self->type = type;
    self->cfg = cfg;
    self->is_joined = FALSE;
    self->srv = srv;
    self->user = srn_chat_add_and_get_user(self, srv->user);
    self->_user = srn_chat_add_and_get_user(self, srv->_user);

    // Init self->ui
    events = &srn_application_get_default()->ui_events;
    self->ui = sui_new_buffer(self, events, self->cfg->ui);

    switch (self->type) {
        case SRN_CHAT_TYPE_SERVER:
            break;
        case SRN_CHAT_TYPE_CHANNEL:
            break;
        case SRN_CHAT_TYPE_DIALOG:
            // Dialog's user_list must contains yourself and your dialogue target
            srn_chat_add_user(self, srn_server_add_and_get_user(srv, self->name));
            break;
        default:
            g_warn_if_reached();
    }

    sui_add_completion(self->ui, self->name);
    for (int i = 0; cmd_binds[i].name != NULL; i++){
        sui_add_completion(self->ui, cmd_binds[i].name);
    }

    return self;
}

void srn_chat_free(SrnChat *self){
    str_assign(&self->name, NULL);

    /* Free extra list: TODO: a better way? */
    relay_decroator_free_list(self);
    nick_filter_free_list(self);
    regex_filter_free_list(self);

    // Free user list, self->user and self->_user also in this list
    g_slist_free_full(self->user_list, (GDestroyNotify)srn_chat_user_free);

    sui_free_buffer(self->ui);
    g_free(self);
}

void srn_chat_set_config(SrnChat *self, SrnChatConfig *cfg){
    sui_buffer_set_config(self->ui, cfg->ui);
    self->cfg = cfg;
}

void srn_chat_set_is_joined(SrnChat *self, bool joined){
    GSList *lst;

    if (self->is_joined == joined){
        return;
    }
    self->is_joined = joined;

    if (!joined){
        lst = self->user_list;
        while (lst){
            SrnChatUser *user;

            user = lst->data;
            srn_chat_user_set_is_joined(user, FALSE);
            lst = g_slist_next(lst);
        }
    }
}

SrnRet srn_chat_add_user(SrnChat *self, SrnServerUser *srv_user){
    GSList *lst;
    SrnChatUser *user;

    lst = self->user_list;
    while (lst) {
        user = lst->data;
        if (user->srv_user == srv_user){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    user = srn_chat_user_new(self, srv_user);
    self->user_list = g_slist_append(self->user_list, user);

    return SRN_OK;
}

SrnChatUser* srn_chat_add_and_get_user(SrnChat *self, SrnServerUser *srv_user){
    srn_chat_add_user(self, srv_user);
    return srn_chat_get_user(self, srv_user->nick);
}

SrnRet srn_chat_rm_user(SrnChat *self, SrnChatUser *user){
    GSList *lst;

    lst = g_slist_find(self->user_list, user);
    if (!lst) {
        return SRN_ERR;
    }
    self->user_list = g_slist_delete_link(self->user_list, lst);

    return SRN_OK;
}


SrnChatUser* srn_chat_get_user(SrnChat *self, const char *nick){
    GSList *lst;
    SrnChatUser *user;

    lst = self->user_list;
    while (lst){
        user = lst->data;
        if (sirc_target_equal(user->srv_user->nick, nick)){
            return user;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}

void srn_chat_add_sent_message(SrnChat *self, const char *content){
    SrnChatUser *user;
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = self->user;
    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_CHAT_LOG;
    msg = srn_message_new(self, user, content, SRN_MESSAGE_SENT);

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        /* Ignore this message */
        goto cleanup;
    }

    if (whether_merge_last_message(self, msg)){
        msg->ui = self->last_msg->ui;
        sui_message_append_message(self->ui, msg->ui, msg->dcontent);
    } else {
        msg->ui = sui_add_sent_msg(self->ui, msg->dcontent);
    }

    if (!msg->ui){
        goto cleanup;
    }

    sui_message_set_time(msg->ui, msg->time);

    append_image(msg);

    add_message(self, msg);

    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_recv_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    dflag = DECORATOR_PANGO_MARKUP | DECORATOR_RELAY | DECORATOR_MENTION;
    if (self->cfg->render_mirc_color) {
        dflag |= DECORATOR_MIRC_COLORIZE;
    } else {
        dflag |= DECORATOR_MIRC_STRIP;
    }
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;

    msg = srn_message_new(self, user, content, SRN_MESSAGE_RECV);
    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    if (whether_merge_last_message(self, msg)){
        msg->ui = self->last_msg->ui;
        sui_message_append_message(self->ui, msg->ui, msg->dcontent);
    } else {
        msg->ui = sui_add_recv_msg(self->ui, msg->dname, msg->role, msg->dcontent);
    }

    if (!msg->ui){
        goto cleanup;
    }

    if (msg->mentioned){
        sui_message_mentioned(msg->ui);
        sui_message_notify(msg->ui);
    } else if (self->type == SRN_CHAT_TYPE_DIALOG){
        sui_message_notify(msg->ui);
    }

    sui_message_set_time(msg->ui, msg->time);

    append_image(msg);

    add_message(self, msg);
    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_notice_message(SrnChat *self, SrnChatUser *user, const char *content){
    srn_chat_add_recv_message(self, user, content);
}

void srn_chat_add_action_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    FilterFlag fflag;
    DecoratorFlag dflag;

    dflag = DECORATOR_PANGO_MARKUP;
    if (self->cfg->render_mirc_color) {
        dflag |= DECORATOR_MIRC_COLORIZE;
    } else {
        dflag |= DECORATOR_MIRC_STRIP;
    }
    fflag = FILTER_CHAT_LOG;

    msg = srn_message_new(self, user, content, SRN_MESSAGE_ACTION);
    if (!user->srv_user->is_me){
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
        char *action_msg = g_strdup_printf(_("*** <b>%1$s</b> %2$s ***"),
                msg->dname, msg->dcontent);
        msg->ui = sui_add_sys_msg(self->ui, action_msg, SYS_MSG_ACTION);
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

    add_message(self, msg);
    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_misc_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = srn_message_new(self, user, content, SRN_MESSAGE_MISC);
    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    msg->ui = sui_add_sys_msg(self->ui, msg->dcontent, SYS_MSG_NORMAL);
    if (!msg->ui){
        goto cleanup;
    }

    add_message(self, msg);
    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_misc_message_fmt(SrnChat *self, SrnChatUser *user, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);
    srn_chat_add_misc_message(self, user, content);
    g_free(content);
}

void srn_chat_add_error_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = srn_message_new(self, user, content, SRN_MESSAGE_ERROR);
    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }
    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    msg->ui = sui_add_sys_msg(self->ui, msg->dcontent, SYS_MSG_ERROR);
    sui_message_notify(msg->ui);

    add_message(self, msg);

    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_error_message_fmt(SrnChat *self, SrnChatUser *user, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);
    srn_chat_add_error_message(self, user, content);
    g_free(content);
}

void srn_chat_set_topic(SrnChat *self, SrnChatUser *user, const char *topic){
    SrnMessage *msg;
    DecoratorFlag dflag;

    dflag = DECORATOR_PANGO_MARKUP;
    if (self->cfg->render_mirc_color) {
        dflag |= DECORATOR_MIRC_COLORIZE;
    } else {
        dflag |= DECORATOR_MIRC_STRIP;
    }

    msg = srn_message_new(self, user, topic, SRN_MESSAGE_UNKNOWN);
    if (decorate_message(msg, dflag, NULL) == SRN_OK){
        sui_set_topic(self->ui, msg->dcontent);
    }
    srn_message_free(msg);
}

void srn_chat_set_topic_setter(SrnChat *self, const char *setter){
    sui_set_topic_setter(self->ui, setter);
}

static void add_message(SrnChat *self, SrnMessage *msg){
    self->msg_list = g_list_append(self->msg_list, msg);
    self->last_msg = msg;
}

static bool whether_merge_last_message(SrnChat *self, SrnMessage *msg){
    SrnMessage *last_msg;

    last_msg = self->last_msg;

    return (last_msg
            && msg->time - last_msg->time < SRN_MESSAGE_MERGE_INTERVAL
            && (!last_msg->mentioned && !msg->mentioned)
            && last_msg->type == msg->type
            && sirc_target_equal(last_msg->user->srv_user->nick, msg->user->srv_user->nick)
            && sirc_target_equal(last_msg->dname, msg->dname));
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
