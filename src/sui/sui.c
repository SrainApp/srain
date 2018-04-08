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

/* @file sui.c
 * @brief Srain UI module interfaces
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-06-29
 */

#include <gtk/gtk.h>
#include <string.h>

#include "sui/sui.h"
#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"
#include "ret.h"

#include "sui_common.h"
#include "sui_app.h"
#include "sui_window.h"
#include "srain_buffer.h"
#include "srain_server_buffer.h"
#include "srain_chat_buffer.h"
#include "srain_channel_buffer.h"
#include "srain_private_buffer.h"
#include "snotify.h"

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiApplication* sui_new_application(const char *id,
        SuiApplicationEvents *events, SuiApplicationConfig *cfg) {
    return sui_application_new(id, events, cfg);
}

void sui_free_application(SuiApplication *app){
    sui_application_quit(app);
}

void sui_run_application(SuiApplication *app, int argc, char *argv[]){
    sui_application_run(app, argc, argv);
}

SuiWindow* sui_new_window(SuiApplication *app, SuiWindowEvents *events,
    SuiWindowConfig *cfg){
    SuiWindow *win;

    win = sui_window_new(app, events, cfg);
    gtk_window_present(GTK_WINDOW(win));

    return win;
}

void sui_free_window(SuiWindow *win){
    g_return_if_fail(win);
    // TODO
}

SuiBuffer* sui_new_server_buffer(const char *srv, void *ctx,
        SuiBufferEvents *events, SuiBufferConfig *cfg){
    SuiBuffer *buf;
    SrainJoinPopover *popover;

    buf = SUI_BUFFER(srain_server_buffer_new(srv, ctx, events, cfg));

    sui_window_add_buffer(sui_get_cur_window(), buf);
    sui_buffer_show_topic(buf, buf->cfg->show_topic);

    popover = sui_window_get_join_popover(sui_get_cur_window());
    srain_join_popover_prepare_model(popover, SRAIN_SERVER_BUFFER(buf));

    return buf;
}

SuiBuffer *sui_new_channel_buffer(SuiBuffer *srv_buf, const char *chan, void *ctx,
        SuiBufferEvents *events, SuiBufferConfig *cfg){
    SuiBuffer *buf;

    g_return_val_if_fail(SRAIN_IS_SERVER_BUFFER(srv_buf), NULL);

    buf = SUI_BUFFER(srain_channel_buffer_new(
            SRAIN_SERVER_BUFFER(srv_buf), chan, ctx, events, cfg));
    sui_window_add_buffer(sui_get_cur_window(), buf);
    srain_server_buffer_add_buffer(SRAIN_SERVER_BUFFER(srv_buf), buf);

    sui_buffer_show_topic(buf, buf->cfg->show_topic);
    srain_chat_buffer_show_user_list(
            SRAIN_CHAT_BUFFER(buf), buf->cfg->show_user_list);

    return buf;
}

SuiBuffer* sui_new_private_buffer(SuiBuffer *srv_buf, const char *nick, void *ctx,
        SuiBufferEvents *events, SuiBufferConfig *cfg){
    SuiBuffer *buf;

    g_return_val_if_fail(SRAIN_IS_SERVER_BUFFER(srv_buf), NULL);

    buf = SUI_BUFFER(srain_private_buffer_new(
            SRAIN_SERVER_BUFFER(srv_buf), nick, ctx, events, cfg));
    sui_window_add_buffer(sui_get_cur_window(), buf);
    srain_server_buffer_add_buffer(SRAIN_SERVER_BUFFER(srv_buf), buf);
    sui_buffer_show_topic(buf, buf->cfg->show_topic);
    srain_chat_buffer_show_user_list(
            SRAIN_CHAT_BUFFER(buf), buf->cfg->show_user_list);

    return buf;
}

void sui_free_buffer(SuiBuffer *buf){
    g_return_if_fail(SUI_IS_BUFFER(buf));

    if (SRAIN_IS_SERVER_BUFFER(buf)) {
        GSList *buf_list;
        SrainServerBuffer *srv_buf;

        srv_buf = SRAIN_SERVER_BUFFER(buf);
        buf_list = srain_server_buffer_get_buffer_list(srv_buf);
        /* A server buffer with non-empty buffer list can not be freed */
        g_return_if_fail(!buf_list || g_slist_length(buf_list) == 0);
    } else if (SRAIN_IS_CHAT_BUFFER(buf)) {
        SrainChatBuffer *chat_buf;
        SrainServerBuffer *srv_buf;

        chat_buf = SRAIN_CHAT_BUFFER(buf);
        srv_buf = SRAIN_SERVER_BUFFER(srain_chat_buffer_get_server_buffer(chat_buf));
        srain_server_buffer_rm_buffer(srv_buf, SUI_BUFFER(chat_buf));
    }

    sui_window_rm_buffer(sui_get_cur_window(), buf);
}

SuiMessage *sui_add_sys_msg(SuiBuffer *buf, const char *msg, SysMsgType type){
    SuiMessage *smsg;
    SuiBuffer *buffer;
    SrainMsgList *list;

    g_return_val_if_fail(buf, NULL);
    g_return_val_if_fail(SUI_IS_BUFFER(buf), NULL);
    g_return_val_if_fail(msg, NULL);

    buffer = buf;

    list = sui_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_sys_msg_new(msg, type);
    sui_message_set_ctx(smsg, buf);
    srain_msg_list_add_message(list, smsg);

    if (type != SYS_MSG_NORMAL){
        sui_window_stack_sidebar_update(sui_get_cur_window(), buffer, NULL, msg);
    }

    return smsg;
}

SuiMessage *sui_add_sent_msg(SuiBuffer *buf, const char *msg){
    SuiBuffer *buffer;
    SrainMsgList *list;
    SuiMessage *smsg;

    g_return_val_if_fail(buf, NULL);
    g_return_val_if_fail(SUI_IS_BUFFER(buf), NULL);
    g_return_val_if_fail(msg, NULL);

    buffer = buf;
    list = sui_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_send_msg_new(msg);
    sui_message_set_ctx(smsg, buf);
    srain_msg_list_add_message(list, smsg);

    sui_window_stack_sidebar_update(sui_get_cur_window(), buffer, _("You"), msg);

    return smsg;
}

SuiMessage *sui_add_recv_msg(SuiBuffer *buf, const char *nick, const char *id,
        const char *msg){
    SuiBuffer *buffer;
    SrainMsgList *list;
    SrainEntryCompletion *comp;
    SuiMessage *smsg;

    g_return_val_if_fail(buf, NULL);
    g_return_val_if_fail(SUI_IS_BUFFER(buf), NULL);
    g_return_val_if_fail(nick, NULL);
    g_return_val_if_fail(msg, NULL);
    if (!id) id = "";

    buffer = buf;
    list = sui_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_recv_msg_new(nick, id, msg);
    sui_message_set_ctx(smsg, buf);
    srain_recv_msg_show_avatar(SRAIN_RECV_MSG(smsg), buf->cfg->show_avatar);
    srain_msg_list_add_message(list, smsg);

    sui_window_stack_sidebar_update(sui_get_cur_window(), buffer, nick, msg);
    if (strlen(id) != 0){
        comp = sui_buffer_get_entry_completion(buffer);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_TMP);
    }

    return smsg;
}

SrnRet sui_add_user(SuiBuffer *buf, const char *nick, UserType type){
    SrnRet ret;
    SrainChatBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(buf, SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);
    g_return_val_if_fail(SRAIN_IS_CHAT_BUFFER(buf), SRN_ERR);

    buffer = SRAIN_CHAT_BUFFER(buf);
    list = srain_chat_buffer_get_user_list(buffer);

    ret = srain_user_list_add(list, nick, type);
    if (RET_IS_OK(ret)){
        comp = sui_buffer_get_entry_completion(SUI_BUFFER(buffer));
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return ret;
}

SrnRet sui_rm_user(SuiBuffer *buf, const char *nick){
    SrnRet ret;
    SrainChatBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(buf, SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);
    g_return_val_if_fail(SRAIN_IS_CHAT_BUFFER(buf), SRN_ERR);

    buffer = SRAIN_CHAT_BUFFER(buf);
    list = srain_chat_buffer_get_user_list(buffer);

    ret = srain_user_list_rm(list, nick);
    if (RET_IS_OK(ret)){
        comp = sui_buffer_get_entry_completion(SUI_BUFFER(buffer));
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return ret;
}

SrnRet sui_ren_user(SuiBuffer *buf, const char *old_nick, const char *new_nick,
        UserType type){
    SrnRet ret;
    SrainChatBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(buf, SRN_ERR);
    g_return_val_if_fail(old_nick, SRN_ERR);
    g_return_val_if_fail(new_nick, SRN_ERR);
    if (!SRAIN_IS_CHAT_BUFFER(buf)){
        return SRN_ERR;
    }

    buffer = SRAIN_CHAT_BUFFER(buf);
    list = srain_chat_buffer_get_user_list(buffer);

    /* Your nick changed */
    if (strcmp(old_nick, sui_buffer_get_nick(SUI_BUFFER(buffer))) == 0){
        sui_buffer_set_nick(SUI_BUFFER(buffer), new_nick);
    }

    ret = srain_user_list_rename(list, old_nick, new_nick, type);
    if (RET_IS_OK(ret)){
        comp = sui_buffer_get_entry_completion(SUI_BUFFER(buffer));
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }

    return ret;
}

void sui_set_topic(SuiBuffer *buf, const char *topic){
    SuiBuffer *buffer;

    g_return_if_fail(buf);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(topic);

    buffer = buf;

    sui_buffer_set_topic(buffer, topic);
}

void sui_set_topic_setter(SuiBuffer *buf, const char *setter){
    SuiBuffer *buffer;

    g_return_if_fail(buf);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(setter);

    buffer = buf;

    sui_buffer_set_topic_setter(buffer, setter);
}

void sui_message_set_ctx(SuiMessage *smsg, void *ctx){
    g_return_if_fail(smsg);

    smsg->ctx = ctx;
}

void *sui_message_get_ctx(SuiMessage *smsg){
    g_return_val_if_fail(smsg, NULL);

    return smsg->ctx;
}

void sui_message_append_message(SuiBuffer *buf, SuiMessage *smsg, const char *msg){
    SuiBuffer *buffer;

    g_return_if_fail(buf);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(smsg);
    g_return_if_fail(msg);

    buffer = buf;

    srain_msg_append_msg(smsg, msg);

    if (SRAIN_IS_RECV_MSG(smsg)){
        sui_window_stack_sidebar_update(sui_get_cur_window(), buffer,
                gtk_label_get_text(SRAIN_RECV_MSG(smsg)->nick_label), msg);
    }
    else if (SRAIN_IS_SEND_MSG(smsg)) {
        sui_window_stack_sidebar_update(sui_get_cur_window(), buffer, _("You"), msg);
    } else {
        WARN_FR("Append message is not available for message %p", smsg);
    }
}

void sui_message_append_image(SuiMessage *smsg, const char *url){
    SrainImageFlag flag;
    SuiBuffer *buf;

    g_return_if_fail(smsg);
    g_return_if_fail(url);
    g_return_if_fail(sui_message_get_ctx(smsg));

    buf = sui_message_get_ctx(smsg);
    flag = SRAIN_IMAGE_ENLARGE | SRAIN_IMAGE_SPININER;

    if (buf->cfg->preview_image){
        flag |= SRAIN_IMAGE_AUTOLOAD;
    } else {
    }


    srain_msg_append_image(smsg, url, flag);
}

void sui_message_mentioned(SuiMessage *smsg){
    g_return_if_fail(smsg);

    srain_msg_list_highlight_message(smsg);
}

void sui_message_set_time(SuiMessage *smsg, time_t time){
    srain_msg_set_time(smsg, time);
}

/**
 * @brief sui_message_noitfy Send a desktop notification, if windows is active,
 *        notification will not be sent.
 *
 * @param smsg
 */
void sui_message_notify(SuiMessage *smsg){
    const char *title;
    const char *msg;
    const char *icon;
    SuiBuffer *buf;

    g_return_if_fail(smsg);

    if (sui_window_is_active(sui_get_cur_window())){
        return;
    }

    buf = sui_message_get_ctx(smsg);
    if (buf && !buf->cfg->notify){
        return;
    }

    title = NULL;
    msg = gtk_label_get_text(smsg->msg_label);
    icon = "im.srain.Srain";

    if (SRAIN_IS_RECV_MSG(smsg)){
        title = gtk_label_get_text(SRAIN_RECV_MSG(smsg)->nick_label);
    }
    else if (SRAIN_IS_SYS_MSG(smsg)){
        SrainSysMsg *ssmsg = SRAIN_SYS_MSG(smsg);
        if (ssmsg->type == SYS_MSG_ERROR){
            title = _("Error");
            icon = "srain-red";
        }
        else if (ssmsg->type == SYS_MSG_ACTION){
            title = _("Action");
        }
    }

    g_return_if_fail(title);

    snotify_notify(title, msg, icon);
    sui_window_tray_icon_stress(sui_get_cur_window(), 1);
}

void sui_add_completion(SuiBuffer *buf, const char *keyword){
    SuiBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(buf);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(keyword);

    buffer = buf;
    comp = sui_buffer_get_entry_completion(buffer);

    srain_entry_completion_add_keyword(comp, keyword, KEYWORD_NORMAL);
}

void sui_rm_completion(SuiBuffer *buf, const char *keyword){
    SuiBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(buf);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(keyword);

    buffer = buf;
    comp = sui_buffer_get_entry_completion(buffer);

    srain_entry_completion_rm_keyword(comp, keyword);
}

void sui_message_box(const char *title, const char *msg){
    GtkMessageDialog *dia;
    char *markuped_msg;

    gtk_init(0, NULL); // FIXME: config

    dia = GTK_MESSAGE_DIALOG(
            gtk_message_dialog_new(GTK_WINDOW(sui_get_cur_window()),
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                NULL
                )
            );

    gtk_window_set_title(GTK_WINDOW(dia), title);
    // TODO: accpet markuped message
    markuped_msg = g_markup_escape_text(msg, -1);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dia), markuped_msg);
    g_free(markuped_msg);

    /* Without this, message dialog cannot be displayed on the center of screen */
    sui_proc_pending_event();

    gtk_dialog_run(GTK_DIALOG(dia));
    gtk_widget_destroy(GTK_WIDGET(dia));
}

void sui_chan_list_start(SuiBuffer *buf){
    g_return_if_fail(buf);
    g_return_if_fail(SRAIN_IS_SERVER_BUFFER(buf));

    srain_server_buffer_start_add_channel(SRAIN_SERVER_BUFFER(buf));
}

void sui_chan_list_add(SuiBuffer *buf, const char *chan, int users,
        const char *topic){
    g_return_if_fail(buf);
    g_return_if_fail(SRAIN_IS_SERVER_BUFFER(buf));
    g_return_if_fail(chan);
    g_return_if_fail(topic);

    srain_server_buffer_add_channel(SRAIN_SERVER_BUFFER(buf),
            chan, users, topic);
    sui_proc_pending_event();
}

void sui_chan_list_end(SuiBuffer *buf){
    g_return_if_fail(buf);
    g_return_if_fail(SRAIN_IS_SERVER_BUFFER(buf));

    srain_server_buffer_end_add_channel(SRAIN_SERVER_BUFFER(buf));
}

void sui_server_list_add(const char *server){
    SrainConnectPopover *popover;

    popover = sui_window_get_connect_popover(sui_get_cur_window());
    srain_connect_popover_add_server(popover, server);
}
