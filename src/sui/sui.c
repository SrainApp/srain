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
#include "sui_common.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_buffer.h"
#include "srain_server_buffer.h"
#include "srain_chat_buffer.h"
#include "srain_channel_buffer.h"
#include "srain_private_buffer.h"
#include "theme.h"
#include "snotify.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"
#include "ret.h"

struct _SuiApplication {
    SrainApp *app;
    SuiApplicationEvents *events;
    SuiApplicationConfig *cfg;

    void *ctx; // FIXME
};

struct _SuiWindow {
    SrainWindow *win;
    SuiWindowEvents*events;
    SuiWindowConfig *cfg;
};

struct _SuiSession{
    SrainBuffer *buffer;

    SuiSessionFlag flag;
    SuiEvents *events;
    SuiConfig *cfg;

    void *ctx;
};

static SuiWindow *sui_win = NULL; // FIXME

SuiApplication* sui_new_application(const char *id,
        SuiApplicationEvents *events, SuiApplicationConfig *cfg) {
    SuiApplication *app;

    app = g_malloc0(sizeof(SuiApplication));
    app->app = srain_app_new(app, id);
    app->events = events;
    app->cfg = cfg;

    return app;
}

void sui_free_application(SuiApplication *app){
    // TODO
}

SuiApplicationEvents* sui_application_get_events(SuiApplication *app){
    return app->events;
}

void* sui_application_get_ctx(SuiApplication *app){
    return app->ctx;
}

void sui_application_set_ctx(SuiApplication *app, void *ctx){
    app->ctx = ctx;
}

void sui_run_application(SuiApplication *app, int argc, char *argv[]){
    // FIXME
    snotify_init();

    if (theme_load(app->cfg->theme) == SRN_ERR){
        char *errmsg;

        errmsg = g_strdup_printf(_("Failed to load theme \"%1$s\""),
                app->cfg->theme);
        sui_message_box(_("Error"), errmsg);
        g_free(errmsg);
    }

    g_application_run(G_APPLICATION(app->app), argc, argv);

    snotify_finalize();
}

SuiWindow* sui_new_window(SuiApplication *app, SuiWindowEvents *events,
        SuiWindowConfig *cfg){
    // Allow only one window for now
    if (!sui_win){
        sui_win = g_malloc0(sizeof(SuiWindow));
        sui_win->win = srain_window_new(app->app, sui_win);
        sui_win->events = events;
        sui_win->cfg = cfg;
        gtk_window_present(GTK_WINDOW(sui_win->win));
        return sui_win;
    }
    return NULL;
}

void sui_free_window(SuiWindow *win){
    g_return_if_fail(win);

    g_free(win); // TODO
}

SuiWindowEvents* sui_window_get_events(SuiWindow *sui) {
    g_return_val_if_fail(sui, NULL);
    return sui->events;
}

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiSession *sui_new_session(SuiEvents *events, SuiConfig *cfg, SuiSessionFlag flag){
    SrnRet ret;
    SuiSession *sui;

    g_return_val_if_fail(events, NULL);
    g_return_val_if_fail(cfg, NULL);

    ret = sui_config_check(cfg);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
        return NULL;
    }

    sui = g_malloc0(sizeof(SuiSession));

    // sui->buffer = NULL; // via g_malloc0()
    sui->flag = flag;
    sui->events = events;
    sui->cfg = cfg;

    return sui;
}

void sui_free_session(SuiSession *sui){
    g_return_if_fail(sui);

    if (sui->buffer){
        sui_end_session(sui);
    }
    g_free(sui);
}

SrnRet sui_server_session(SuiSession *sui, const char *srv){
    SrainServerBuffer *buffer;
    SrainJoinPopover *popover;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(srv, SRN_ERR);

    buffer = srain_server_buffer_new(sui, srv);
    if (!buffer) {
        return RET_ERR(_("Failed to create server buffer"));
    }
    sui->buffer = SRAIN_BUFFER(buffer);

    srain_window_add_buffer(sui_win->win, sui->buffer);
    srain_buffer_show_topic(sui->buffer, sui->cfg->show_topic);

    popover = srain_window_get_join_popover(sui_win->win);
    srain_join_popover_prepare_model(popover, buffer);

    return SRN_OK;
}

SrnRet sui_channel_session(SuiSession *sui, SuiSession *srv_sui, const char *chan){
    SrainChannelBuffer *buffer;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(srv_sui, SRN_ERR);
    g_return_val_if_fail(SRAIN_IS_SERVER_BUFFER(srv_sui->buffer), SRN_ERR);
    g_return_val_if_fail(chan, SRN_ERR);

    buffer = srain_channel_buffer_new(
            sui, SRAIN_SERVER_BUFFER(srv_sui->buffer), chan);
    if (!buffer) {
        return RET_ERR(_("Failed to create channel buffer"));
    }

    sui->buffer = SRAIN_BUFFER(buffer);
    srain_window_add_buffer(sui_win->win, SRAIN_BUFFER(sui->buffer));
    srain_server_buffer_add_buffer(
            SRAIN_SERVER_BUFFER(srv_sui->buffer), SRAIN_BUFFER(buffer));
    srain_buffer_show_topic(SRAIN_BUFFER(sui->buffer), sui->cfg->show_topic);
    srain_chat_buffer_show_user_list(
            SRAIN_CHAT_BUFFER(sui->buffer), sui->cfg->show_user_list);

    return SRN_OK;
}

SrnRet sui_private_session(SuiSession *sui, SuiSession *srv_sui, const char *nick){
    SrainPrivateBuffer *buffer;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(srv_sui, SRN_ERR);
    g_return_val_if_fail(SRAIN_IS_SERVER_BUFFER(srv_sui->buffer), SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);

    buffer = srain_private_buffer_new(
            sui, SRAIN_SERVER_BUFFER(srv_sui->buffer), nick);
    if (!buffer) {
        return RET_ERR(_("Failed to create private buffer"));
    }

    sui->buffer = SRAIN_BUFFER(buffer);
    srain_window_add_buffer(sui_win->win, SRAIN_BUFFER(sui->buffer));
    srain_server_buffer_add_buffer(
            SRAIN_SERVER_BUFFER(srv_sui->buffer), SRAIN_BUFFER(buffer));
    srain_buffer_show_topic(SRAIN_BUFFER(sui->buffer), sui->cfg->show_topic);
    srain_chat_buffer_show_user_list(
            SRAIN_CHAT_BUFFER(sui->buffer), sui->cfg->show_user_list);

    return SRN_OK;
}

void sui_end_session(SuiSession *sui){
    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_BUFFER(sui->buffer));

    if (SRAIN_IS_SERVER_BUFFER(sui->buffer)) {
        GSList *buffer_list;
        SrainServerBuffer *srv_buf;

        srv_buf = SRAIN_SERVER_BUFFER(sui->buffer);
        buffer_list = srain_server_buffer_get_buffer_list(srv_buf);
        /* A server buffer with non-empty buffer list can not be freed */
        g_return_if_fail(!buffer_list || g_slist_length(buffer_list) == 0);
    } else if (SRAIN_IS_CHAT_BUFFER(sui->buffer)) {
        SrainChatBuffer *chat_buf;
        SrainServerBuffer *srv_buf;

        chat_buf = SRAIN_CHAT_BUFFER(sui->buffer);
        srv_buf = SRAIN_SERVER_BUFFER(srain_chat_buffer_get_server_buffer(chat_buf));
        srain_server_buffer_rm_buffer(srv_buf, SRAIN_BUFFER(chat_buf));
    }

    srain_window_rm_buffer(sui_win->win, sui->buffer);
    // FIXME: unref?
    sui->buffer = NULL;
}

SuiSessionFlag sui_get_flag(SuiSession *sui){
    /* Don't return SRN_ERR(-1 0xffffffff) */
    g_return_val_if_fail(sui, 0);

    return sui->flag;
}

SuiEvents *sui_get_events(SuiSession *sui){
    g_return_val_if_fail(sui, NULL);

    return sui->events;
}

SuiConfig *sui_get_cfg(SuiSession *sui){
    g_return_val_if_fail(sui, NULL);

    return sui->cfg;
}

void* sui_get_ctx(SuiSession *sui){
    g_return_val_if_fail(sui, NULL);

    return sui->ctx;
}

void sui_set_ctx(SuiSession *sui, void *ctx){
    g_return_if_fail(sui);

    sui->ctx = ctx;
}

void sui_set_name(SuiSession *sui, const char *name){
    g_return_if_fail(sui);
    g_return_if_fail(name);

    srain_buffer_set_name(sui->buffer, name);
}

void sui_set_remark(SuiSession *sui, const char *remark){
    g_return_if_fail(sui);
    g_return_if_fail(remark);

    srain_buffer_set_remark(sui->buffer, remark);
}

SuiMessage *sui_add_sys_msg(SuiSession *sui, const char *msg, SysMsgType type){
    SuiMessage *smsg;
    SrainBuffer *buffer;
    SrainMsgList *list;

    g_return_val_if_fail(sui, NULL);
    g_return_val_if_fail(SRAIN_IS_BUFFER(sui->buffer), NULL);
    g_return_val_if_fail(msg, NULL);

    buffer = sui->buffer;

    list = srain_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_sys_msg_new(msg, type);
    sui_message_set_ctx(smsg, sui);
    srain_msg_list_add_message(list, smsg);

    if (type != SYS_MSG_NORMAL){
        srain_window_stack_sidebar_update(sui_win->win, buffer, NULL, msg);
    }

    return smsg;
}

SuiMessage *sui_add_sent_msg(SuiSession *sui, const char *msg){
    SrainBuffer *buffer;
    SrainMsgList *list;
    SuiMessage *smsg;

    g_return_val_if_fail(sui, NULL);
    g_return_val_if_fail(SRAIN_IS_BUFFER(sui->buffer), NULL);
    g_return_val_if_fail(msg, NULL);

    buffer = sui->buffer;
    list = srain_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_send_msg_new(msg);
    sui_message_set_ctx(smsg, sui);
    srain_msg_list_add_message(list, smsg);

    srain_window_stack_sidebar_update(sui_win->win, buffer, _("You"), msg);

    return smsg;
}

SuiMessage *sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id,
        const char *msg){
    SrainBuffer *buffer;
    SrainMsgList *list;
    SrainEntryCompletion *comp;
    SuiMessage *smsg;

    g_return_val_if_fail(sui, NULL);
    g_return_val_if_fail(SRAIN_IS_BUFFER(sui->buffer), NULL);
    g_return_val_if_fail(nick, NULL);
    g_return_val_if_fail(msg, NULL);
    if (!id) id = "";

    buffer = sui->buffer;
    list = srain_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_recv_msg_new(nick, id, msg);
    sui_message_set_ctx(smsg, sui);
    srain_recv_msg_show_avatar(SRAIN_RECV_MSG(smsg), sui->cfg->show_avatar);
    srain_msg_list_add_message(list, smsg);

    srain_window_stack_sidebar_update(sui_win->win, buffer, nick, msg);
    if (strlen(id) != 0){
        comp = srain_buffer_get_entry_completion(buffer);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_TMP);
    }

    return smsg;
}

SrnRet sui_add_user(SuiSession *sui, const char *nick, UserType type){
    SrnRet ret;
    SrainChatBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);
    g_return_val_if_fail(SRAIN_IS_CHAT_BUFFER(sui->buffer), SRN_ERR);

    buffer = SRAIN_CHAT_BUFFER(sui->buffer);
    list = srain_chat_buffer_get_user_list(buffer);

    ret = srain_user_list_add(list, nick, type);
    if (RET_IS_OK(ret)){
        comp = srain_buffer_get_entry_completion(SRAIN_BUFFER(buffer));
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return ret;
}

SrnRet sui_rm_user(SuiSession *sui, const char *nick){
    SrnRet ret;
    SrainChatBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);
    g_return_val_if_fail(SRAIN_IS_CHAT_BUFFER(sui->buffer), SRN_ERR);

    buffer = SRAIN_CHAT_BUFFER(sui->buffer);
    list = srain_chat_buffer_get_user_list(buffer);

    ret = srain_user_list_rm(list, nick);
    if (RET_IS_OK(ret)){
        comp = srain_buffer_get_entry_completion(SRAIN_BUFFER(buffer));
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return ret;
}

SrnRet sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick,
        UserType type){
    SrnRet ret;
    SrainChatBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(old_nick, SRN_ERR);
    g_return_val_if_fail(new_nick, SRN_ERR);
    if (!SRAIN_IS_CHAT_BUFFER(sui->buffer)){
        return SRN_ERR;
    }

    buffer = SRAIN_CHAT_BUFFER(sui->buffer);
    list = srain_chat_buffer_get_user_list(buffer);

    /* Your nick changed */
    if (strcmp(old_nick, srain_buffer_get_nick(SRAIN_BUFFER(buffer))) == 0){
        srain_buffer_set_nick(SRAIN_BUFFER(buffer), new_nick);
    }

    ret = srain_user_list_rename(list, old_nick, new_nick, type);
    if (RET_IS_OK(ret)){
        comp = srain_buffer_get_entry_completion(SRAIN_BUFFER(buffer));
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }

    return ret;
}

void sui_set_topic(SuiSession *sui, const char *topic){
    SrainBuffer *buffer;

    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_BUFFER(sui->buffer));
    g_return_if_fail(topic);

    buffer = sui->buffer;

    srain_buffer_set_topic(buffer, topic);
}

void sui_set_topic_setter(SuiSession *sui, const char *setter){
    SrainBuffer *buffer;

    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_BUFFER(sui->buffer));
    g_return_if_fail(setter);

    buffer = sui->buffer;

    srain_buffer_set_topic_setter(buffer, setter);
}

void sui_message_set_ctx(SuiMessage *smsg, void *ctx){
    g_return_if_fail(smsg);

    smsg->ctx = ctx;
}

void *sui_message_get_ctx(SuiMessage *smsg){
    g_return_val_if_fail(smsg, NULL);

    return smsg->ctx;
}

void sui_message_append_message(SuiSession *sui, SuiMessage *smsg, const char *msg){
    SrainBuffer *buffer;

    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_BUFFER(sui->buffer));
    g_return_if_fail(smsg);
    g_return_if_fail(msg);

    buffer = sui->buffer;

    srain_msg_append_msg(smsg, msg);

    if (SRAIN_IS_RECV_MSG(smsg)){
        srain_window_stack_sidebar_update(sui_win->win, buffer,
                gtk_label_get_text(SRAIN_RECV_MSG(smsg)->nick_label), msg);
    }
    else if (SRAIN_IS_SEND_MSG(smsg)) {
        srain_window_stack_sidebar_update(sui_win->win, buffer, _("You"), msg);
    } else {
        WARN_FR("Append message is not available for message %p", smsg);
    }
}

void sui_message_append_image(SuiMessage *smsg, const char *url){
    SrainImageFlag flag;
    SuiSession *sui;

    g_return_if_fail(smsg);
    g_return_if_fail(url);
    g_return_if_fail(sui_message_get_ctx(smsg));

    sui = sui_message_get_ctx(smsg);
    flag = SRAIN_IMAGE_ENLARGE | SRAIN_IMAGE_SPININER;

    if (sui->cfg->preview_image){
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
    SuiSession *sui;

    g_return_if_fail(smsg);

    if (srain_window_is_active(sui_win->win)){
        return;
    }

    sui = sui_message_get_ctx(smsg);
    if (sui && !sui->cfg->notify){
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
    srain_window_tray_icon_stress(sui_win->win, 1);
}

void sui_add_completion(SuiSession *sui, const char *keyword){
    SrainBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_BUFFER(sui->buffer));
    g_return_if_fail(keyword);

    buffer = sui->buffer;
    comp = srain_buffer_get_entry_completion(buffer);

    srain_entry_completion_add_keyword(comp, keyword, KEYWORD_NORMAL);
}

void sui_rm_completion(SuiSession *sui, const char *keyword){
    SrainBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_BUFFER(sui->buffer));
    g_return_if_fail(keyword);

    buffer = sui->buffer;
    comp = srain_buffer_get_entry_completion(buffer);

    srain_entry_completion_rm_keyword(comp, keyword);
}

void sui_message_box(const char *title, const char *msg){
    GtkMessageDialog *dia;
    char *markuped_msg;

    gtk_init(0, NULL); // FIXME: config

    dia = GTK_MESSAGE_DIALOG(
            gtk_message_dialog_new(GTK_WINDOW(sui_win->win),
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

void sui_chan_list_start(SuiSession *sui){
    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_SERVER_BUFFER(sui->buffer));

    srain_server_buffer_start_add_channel(SRAIN_SERVER_BUFFER(sui->buffer));
}

void sui_chan_list_add(SuiSession *sui, const char *chan, int users,
        const char *topic){
    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_SERVER_BUFFER(sui->buffer));
    g_return_if_fail(chan);
    g_return_if_fail(topic);

    srain_server_buffer_add_channel(SRAIN_SERVER_BUFFER(sui->buffer),
            chan, users, topic);
    sui_proc_pending_event();
}

void sui_chan_list_end(SuiSession *sui){
    g_return_if_fail(sui);
    g_return_if_fail(SRAIN_IS_SERVER_BUFFER(sui->buffer));

    srain_server_buffer_end_add_channel(SRAIN_SERVER_BUFFER(sui->buffer));
}

void sui_server_list_add(const char *server){
    SrainConnectPopover *popover;

    popover = srain_window_get_connect_popover(sui_win->win);
    srain_connect_popover_add_server(popover, server);
}
