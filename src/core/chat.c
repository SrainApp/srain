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
#include "render/render.h"
#include "filter/filter.h"
#include "i18n.h"
#include "command.h"
#include "log.h"
#include "utils.h"
#include "config/config.h"

#include "sirc/sirc.h"

extern SrnCommandBind cmd_binds[];

static void add_message(SrnChat *self, SrnMessage *msg);
static void init_extra_data(SrnChat *self);
static void finalize_extra_data(SrnChat *self);

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

    init_extra_data(self);

    return self;
}

void srn_chat_free(SrnChat *self){
    str_assign(&self->name, NULL);

    // Free user list, self->user and self->_user also in this list
    g_list_free_full(self->user_list, (GDestroyNotify)srn_chat_user_free);

    sui_free_buffer(self->ui);

    finalize_extra_data(self);

    g_free(self);
}

void srn_chat_set_config(SrnChat *self, SrnChatConfig *cfg){
    sui_buffer_set_config(self->ui, cfg->ui);
    self->cfg = cfg;
}

void srn_chat_set_is_joined(SrnChat *self, bool joined){
    GList *lst;

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
            lst = g_list_next(lst);
        }
    }
}

SrnRet srn_chat_add_user(SrnChat *self, SrnServerUser *srv_user){
    GList *lst;
    SrnChatUser *user;

    lst = self->user_list;
    while (lst) {
        user = lst->data;
        if (user->srv_user == srv_user){
            return SRN_ERR;
        }
        lst = g_list_next(lst);
    }

    user = srn_chat_user_new(self, srv_user);
    self->user_list = g_list_append(self->user_list, user);

    return SRN_OK;
}

SrnChatUser* srn_chat_add_and_get_user(SrnChat *self, SrnServerUser *srv_user){
    srn_chat_add_user(self, srv_user);
    return srn_chat_get_user(self, srv_user->nick);
}

SrnRet srn_chat_rm_user(SrnChat *self, SrnChatUser *user){
    GList *lst;

    lst = g_list_find(self->user_list, user);
    if (!lst) {
        return SRN_ERR;
    }
    self->user_list = g_list_delete_link(self->user_list, lst);

    return SRN_OK;
}


SrnChatUser* srn_chat_get_user(SrnChat *self, const char *nick){
    GList *lst;
    SrnChatUser *user;

    lst = self->user_list;
    while (lst){
        user = lst->data;
        if (sirc_target_equal(user->srv_user->nick, nick)){
            return user;
        }
        lst = g_list_next(lst);
    }

    return NULL;
}

void srn_chat_add_sent_message(SrnChat *self, const char *content){
    SrnChatUser *user;
    SrnMessage *msg;
    SrnRenderFlags rflags;
    SrnFilterFlags fflags;

    user = self->user;
    rflags = SRN_RENDER_FLAG_URL;
    fflags = SRN_FILTER_FLAG_LOG;
    msg = srn_message_new(self, user, content, SRN_MESSAGE_TYPE_SENT);

    if (srn_render_message(msg, rflags) != SRN_OK){
        goto cleanup;
    }
    if (!srn_filter_message(msg, fflags)){
        /* Ignore this message */
        goto cleanup;
    }

    add_message(self, msg);

    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_recv_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    SrnRenderFlags rflags;
    SrnFilterFlags fflags;

    rflags = SRN_RENDER_FLAG_URL | SRN_RENDER_FLAG_RELAY | SRN_RENDER_FLAG_MENTION;
    if (self->cfg->render_mirc_color) {
        rflags |= SRN_RENDER_FLAG_MIRC_COLORIZE;
    } else {
        rflags |= SRN_RENDER_FLAG_MIRC_STRIP;
    }
    fflags = SRN_FILTER_FLAG_USER | SRN_FILTER_FLAG_REGEX | SRN_FILTER_FLAG_LOG;

    msg = srn_message_new(self, user, content, SRN_MESSAGE_TYPE_RECV);
    if (srn_render_message(msg, rflags) != SRN_OK){
        goto cleanup;
    }
    if (!srn_filter_message(msg, fflags)){
        goto cleanup;
    }

    add_message(self, msg);

    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_notice_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    SrnRenderFlags rflags;
    SrnFilterFlags fflags;

    rflags = SRN_RENDER_FLAG_URL | SRN_RENDER_FLAG_RELAY | SRN_RENDER_FLAG_MENTION;
    if (self->cfg->render_mirc_color) {
        rflags |= SRN_RENDER_FLAG_MIRC_COLORIZE;
    } else {
        rflags |= SRN_RENDER_FLAG_MIRC_STRIP;
    }
    fflags = SRN_FILTER_FLAG_USER | SRN_FILTER_FLAG_REGEX | SRN_FILTER_FLAG_LOG;

    msg = srn_message_new(self, user, content, SRN_MESSAGE_TYPE_NOTICE);
    if (srn_render_message(msg, rflags) != SRN_OK){
        goto cleanup;
    }
    if (!srn_filter_message(msg, fflags)){
        goto cleanup;
    }

    add_message(self, msg);

    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_action_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    SrnFilterFlags fflags;
    SrnRenderFlags rflags;

    rflags = SRN_RENDER_FLAG_URL;
    if (self->cfg->render_mirc_color) {
        rflags |= SRN_RENDER_FLAG_MIRC_COLORIZE;
    } else {
        rflags |= SRN_RENDER_FLAG_MIRC_STRIP;
    }
    fflags = SRN_FILTER_FLAG_LOG;

    msg = srn_message_new(self, user, content, SRN_MESSAGE_TYPE_ACTION);
    if (!user->srv_user->is_me){
        fflags |= SRN_FILTER_FLAG_USER | SRN_FILTER_FLAG_REGEX;
        rflags |= SRN_RENDER_FLAG_RELAY | SRN_RENDER_FLAG_MENTION;
    }
    if (srn_render_message(msg, rflags) != SRN_OK){
        goto cleanup;
    }
    if (!srn_filter_message(msg, fflags)){
        goto cleanup;
    }

    if (!msg->ui){
        goto cleanup;
    }

    add_message(self, msg);

    return;

cleanup:
    srn_message_free(msg);
}

void srn_chat_add_misc_message(SrnChat *self, SrnChatUser *user, const char *content){
    SrnMessage *msg;
    SrnRenderFlags rflags;
    SrnFilterFlags fflags;

    rflags = SRN_RENDER_FLAG_URL;
    fflags = SRN_FILTER_FLAG_USER | SRN_FILTER_FLAG_REGEX | SRN_FILTER_FLAG_LOG;
    msg = srn_message_new(self, user, content, SRN_MESSAGE_TYPE_MISC);
    if (srn_render_message(msg, rflags) != SRN_OK){
        goto cleanup;
    }
    if (!srn_filter_message(msg, fflags)){
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
    SrnRenderFlags rflags;
    SrnFilterFlags fflags;

    rflags = SRN_RENDER_FLAG_URL;
    fflags = SRN_FILTER_FLAG_USER | SRN_FILTER_FLAG_REGEX | SRN_FILTER_FLAG_LOG;
    msg = srn_message_new(self, user, content, SRN_MESSAGE_TYPE_ERROR);
    if (srn_render_message(msg, rflags) != SRN_OK){
        goto cleanup;
    }
    if (!srn_filter_message(msg, fflags)){
        goto cleanup;
    }

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
    SrnRenderFlags rflags;

    rflags = SRN_RENDER_FLAG_URL;
    if (self->cfg->render_mirc_color) {
        rflags |= SRN_RENDER_FLAG_MIRC_COLORIZE;
    } else {
        rflags |= SRN_RENDER_FLAG_MIRC_STRIP;
    }

    msg = srn_message_new(self, user, topic, SRN_MESSAGE_TYPE_UNKNOWN);
    if (srn_render_message(msg, rflags) == SRN_OK){
        sui_set_topic(self->ui, msg->rendered_content);
    }
    srn_message_free(msg);
}

void srn_chat_set_topic_setter(SrnChat *self, const char *setter){
    sui_set_topic_setter(self->ui, setter);
}

void srn_chat_set_extra_data(SrnChat *self, const char *key, void *val,
        GDestroyNotify val_destory_func) {
    g_return_if_fail(key);
    g_return_if_fail(val);

    g_return_if_fail(!g_hash_table_contains(self->extra_data_table, key));
    g_return_if_fail(!g_hash_table_contains(self->extra_destory_func_table, key));

    g_hash_table_insert(self->extra_data_table, (gpointer)key, val);
    g_hash_table_insert(self->extra_destory_func_table, (gpointer)key,
            val_destory_func);
}

void* srn_chat_get_extra_data(SrnChat *self, const char *key) {
    return g_hash_table_lookup(self->extra_data_table, key);
}

static void add_message(SrnChat *self, SrnMessage *msg){
    self->msg_list = g_list_append(self->msg_list, msg);
    self->last_msg = msg;

    sui_buffer_add_message(self->ui, msg->ui);
    if (msg->mentioned
            || self->type == SRN_CHAT_TYPE_DIALOG
            || msg->type == SRN_MESSAGE_TYPE_NOTICE
            || msg->type == SRN_MESSAGE_TYPE_ERROR){
        sui_notify_message(msg->ui);
    }
}

static void init_extra_data(SrnChat *self) {
    self->extra_data_table = g_hash_table_new(g_str_hash, g_str_equal);
    self->extra_destory_func_table = g_hash_table_new(g_str_hash, g_str_equal);
}

static void finalize_extra_data(SrnChat *self) {
    gpointer key, val;
    GHashTableIter iter;

    // Free all extra data via destory func
    g_hash_table_iter_init(&iter, self->extra_data_table);
    while (g_hash_table_iter_next(&iter, &key, &val)){
        GDestroyNotify func;

        func = g_hash_table_lookup(self->extra_data_table, key);
        if (func) {
            func(val);
        }
    }

    g_hash_table_destroy(self->extra_data_table);
    g_hash_table_destroy(self->extra_destory_func_table);
}
