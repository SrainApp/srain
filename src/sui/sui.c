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

#include "core/core.h"
#include "sui/sui.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"
#include "ret.h"

#include "sui_common.h"
#include "sui_app.h"
#include "sui_window.h"
#include "sui_buffer.h"
#include "sui_server_buffer.h"
#include "sui_chat_buffer.h"
#include "sui_channel_buffer.h"
#include "sui_dialog_buffer.h"
#include "sui_message.h"
#include "sui_misc_message.h"
#include "sui_send_message.h"
#include "sui_recv_message.h"

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiApplication* sui_new_application(const char *id, void *ctx,
        SuiApplicationEvents *events, SuiApplicationConfig *cfg) {
    return sui_application_new(id, ctx, events, cfg);
}

void sui_free_application(SuiApplication *app){
    sui_application_exit(app);
}

void sui_run_application(SuiApplication *app, int argc, char *argv[]){
    sui_application_run(app, argc, argv);
}

SuiWindow* sui_new_window(SuiApplication *app, SuiWindowEvents *events){
    SuiWindow *win;
    SuiWindowConfig *cfg;
    SuiApplicationConfig *app_cfg;

    cfg = sui_window_config_new();
    app_cfg = sui_application_get_config(app);
    *cfg = app_cfg->window; // Copy window config

    win = sui_window_new(app, events, cfg);
    gtk_window_present(GTK_WINDOW(win));

    return win;
}

void sui_free_window(SuiWindow *win){
    g_return_if_fail(win);
    // TODO
}

SuiBuffer* sui_new_buffer(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg){
    SuiBuffer *buf;
    SrnChat *chat;

    // FIXME: dont use core's type here?
    chat = ctx;
    switch (chat->type) {
        case SRN_CHAT_TYPE_SERVER:
            buf = SUI_BUFFER(sui_server_buffer_new(ctx, events, cfg));
            break;
        case SRN_CHAT_TYPE_CHANNEL:
            buf = SUI_BUFFER(sui_channel_buffer_new(ctx, events, cfg));
            break;
        case SRN_CHAT_TYPE_DIALOG:
            buf = SUI_BUFFER(sui_dialog_buffer_new(ctx, events, cfg));
            break;
        default:
            g_return_val_if_reached(NULL);
    }
    // FIXME: Wrong usage of sui_common_get_cur_window()
    sui_window_add_buffer(sui_common_get_cur_window(), buf);

    return buf;
}

void sui_free_buffer(SuiBuffer *buf){
    g_return_if_fail(SUI_IS_BUFFER(buf));
    // FIXME: Wrong usage of sui_common_get_cur_window()
    sui_window_rm_buffer(sui_common_get_cur_window(), buf);
}

void sui_activate_buffer(SuiBuffer *buf){
    g_return_if_fail(SUI_IS_BUFFER(buf));
    sui_window_set_cur_buffer(sui_common_get_cur_window(), buf);
}

void sui_buffer_add_message(SuiBuffer *buf, SuiMessage *msg){
    GType type;
    SuiWindow *win;
    SuiSideBar *sidebar;
    SuiSideBarItem *item;
    SuiMessageList *list;

    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(SUI_IS_MESSAGE(msg));

    /* Add message */
    sui_message_set_buffer(msg, buf);
    sui_message_update(msg);
    list = sui_buffer_get_message_list(buf);
    type = G_OBJECT_TYPE(msg);
    if (type == SUI_TYPE_MISC_MESSAGE){
        sui_message_list_add_message(list, msg, GTK_ALIGN_CENTER);
    } else if (type == SUI_TYPE_SEND_MESSAGE){
        sui_message_list_add_message(list, msg, GTK_ALIGN_END);
    } else if (type == SUI_TYPE_RECV_MESSAGE){
        sui_message_list_add_message(list, msg, GTK_ALIGN_START);
    } else {
        g_warn_if_reached();
    }

    /* Update side bar */
    win = SUI_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(buf)));
    g_return_if_fail(SUI_IS_WINDOW(win));

    sidebar = sui_window_get_side_bar(win);
    item = sui_side_bar_get_item(sidebar, buf);
    sui_message_update_side_bar_item(msg, item);

    if (buf == sui_common_get_cur_buffer()){
        // Don't show counter while buffer is active
        sui_side_bar_item_clear_count(item);
    }
}

void sui_buffer_clear_message(SuiBuffer *buf){
    SuiWindow *win;
    SuiSideBar *sidebar;
    SuiSideBarItem *item;
    SuiMessageList *list;

    g_return_if_fail(SUI_IS_BUFFER(buf));

    /* Clear messages */
    list = sui_buffer_get_message_list(buf);
    sui_message_list_clear_message(list);

    /* Update side bar */
    win = SUI_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(buf)));
    g_return_if_fail(SUI_IS_WINDOW(win));

    sidebar = sui_window_get_side_bar(win);
    item = sui_side_bar_get_item(sidebar, buf);
    sui_side_bar_item_update(item, "", "");
    sui_side_bar_item_clear_count(item);
}

void sui_free_message(SuiMessage *msg){
    // TODO
}

/**
 * @brief ``sui_notify_message`` sends a notification about the ``msg`` as
 * appropriate.
 *
 * @param msg
 */
void sui_notify_message(SuiMessage *msg){
    bool in_app; // Whether a in-app notification
    SuiApplication *app;
    SuiWindow *win;
    SuiBuffer *buf;
    SuiBufferConfig *buf_cfg;
    SuiNotification *notif;

    g_return_if_fail(SUI_IS_MESSAGE(msg));
    in_app = FALSE;
    app = sui_application_get_instance();
    g_return_if_fail(SUI_IS_APPLICATION(app));
    win = sui_application_get_cur_window(app);
    g_return_if_fail(SUI_IS_WINDOW(win));
    buf = sui_message_get_buffer(msg);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    buf_cfg = sui_buffer_get_config(buf);

    if (!buf_cfg->notify){
        DBG_FR("Notification is disabled");
        return;
    }
    if (sui_window_is_active(win)){
        DBG_FR("Window is active");
        if (sui_window_get_cur_buffer(win) == buf){
            DBG_FR("Buffer is active");
            return;
        }
        in_app = TRUE;
    }

    notif = sui_message_new_notification(msg);
    if (in_app) {
        // TODO: In-app notification support
    } else {
        sui_application_highlight_tray_icon(app, TRUE);
        sui_application_send_notification(app, notif);
    }

    sui_notification_free(notif);
}

SuiMessage *sui_new_misc_message(void *ctx, SuiMiscMessageStyle style){
    return SUI_MESSAGE(sui_misc_message_new(ctx, style));
}

SuiMessage *sui_new_send_message(void *ctx){
    return SUI_MESSAGE(sui_send_message_new(ctx));
}

SuiMessage *sui_new_recv_message(void *ctx){
    return SUI_MESSAGE(sui_recv_message_new(ctx));
}

SuiUser* sui_new_user(void *ctx){
    return sui_user_new(ctx);
}

void sui_free_user(SuiUser *user){
    sui_user_free(user);
}

void sui_update_user(SuiBuffer *buf, SuiUser *user){
    g_return_if_fail(SUI_IS_CHAT_BUFFER(buf));
    g_return_if_fail(user);

    sui_user_list_update_user(
            sui_chat_buffer_get_user_list(SUI_CHAT_BUFFER(buf)), user);
}

void sui_add_user(SuiBuffer *buf, SuiUser *user){
    SuiChatBuffer *chat_buf;
    SuiUserList *list;

    g_return_if_fail(SUI_IS_CHAT_BUFFER(buf));
    g_return_if_fail(user);

    chat_buf = SUI_CHAT_BUFFER(buf);
    list = sui_chat_buffer_get_user_list(chat_buf);

    sui_user_list_add_user(list, user);
}

void sui_rm_user(SuiBuffer *buf, SuiUser *user){
    SuiChatBuffer *chat_buf;
    SuiUserList *list;

    g_return_if_fail(SUI_IS_CHAT_BUFFER(buf));
    g_return_if_fail(user);

    chat_buf = SUI_CHAT_BUFFER(buf);
    list = sui_chat_buffer_get_user_list(chat_buf);

    sui_user_list_rm_user(list, user);
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

void sui_message_box(const char *title, const char *msg){
    GtkMessageDialog *dia;
    char *markuped_msg;

    gtk_init(0, NULL); // FIXME: config

    dia = GTK_MESSAGE_DIALOG(
            gtk_message_dialog_new(GTK_WINDOW(sui_common_get_cur_window()),
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
    SuiServerBuffer *srv_buf;
    SuiJoinPanel *panel;

    g_return_if_fail(SUI_IS_SERVER_BUFFER(buf));
    srv_buf = SUI_SERVER_BUFFER(buf);
    panel = sui_server_buffer_get_join_panel(srv_buf);
    g_return_if_fail(!sui_join_panel_get_is_adding(panel));

    sui_join_panel_set_is_adding(panel, TRUE);
    sui_server_buffer_clear_channel(srv_buf);
}

void sui_chan_list_add(SuiBuffer *buf, const char *chan, int users,
        const char *topic){
    SuiServerBuffer *srv_buf;
    SuiJoinPanel *panel;

    g_return_if_fail(SUI_IS_SERVER_BUFFER(buf));
    g_return_if_fail(chan);
    g_return_if_fail(topic);
    srv_buf = SUI_SERVER_BUFFER(buf);
    panel = sui_server_buffer_get_join_panel(srv_buf);
    g_return_if_fail(sui_join_panel_get_is_adding(panel));

    sui_server_buffer_add_channel(srv_buf, chan, users, topic);
    sui_proc_pending_event();
}

void sui_chan_list_end(SuiBuffer *buf){
    SuiServerBuffer *srv_buf;
    SuiJoinPanel *panel;

    g_return_if_fail(SUI_IS_SERVER_BUFFER(buf));
    srv_buf = SUI_SERVER_BUFFER(buf);
    panel = sui_server_buffer_get_join_panel(srv_buf);
    g_return_if_fail(sui_join_panel_get_is_adding(panel));

    sui_join_panel_set_is_adding(panel, FALSE);
}
