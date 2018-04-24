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
#include "sui_buffer.h"
#include "sui_message.h"
#include "sui_misc_message.h"
#include "sui_send_message.h"
#include "sui_recv_message.h"
#include "snotify.h"

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiApplication* sui_new_application(const char *id, void *ctx,
        SuiApplicationEvents *events, SuiApplicationConfig *cfg) {
    return sui_application_new(id, ctx, events, cfg);
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

SuiBuffer* sui_new_buffer(void *ctx, SuiBufferEvents *events, SuiBufferConfig *cfg){
    SuiBuffer *buf;

    buf = sui_buffer_new(ctx, events, cfg);
    sui_window_add_buffer(sui_get_cur_window(), buf);

    return buf;
}

void sui_free_buffer(SuiBuffer *buf){
    g_return_if_fail(SUI_IS_BUFFER(buf));
    sui_window_rm_buffer(sui_get_cur_window(), buf);
}

void sui_buffer_add_message(SuiBuffer *buf, SuiMessage *msg){
    SrainMsgList *list;

    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(SUI_IS_MESSAGE(msg));

    sui_message_set_buffer(msg, buf);
    list = sui_buffer_get_msg_list(buf);
    srain_msg_list_add_message(list, msg);
}

SuiMessage *sui_new_misc_message(void *ctx, SuiMiscMessageStyle style){
    return SUI_MESSAGE(sui_misc_message_new(ctx, style));
}

SuiMessage *sui_new_send_message(void *ctx){
    return SUI_MESSAGE(sui_send_message_new(ctx));
}

SuiMessage *sui_new_recv_message(void *ctx){
    SuiRecvMessage *srmsg;

    srmsg = sui_recv_message_new(ctx);
    sui_message_update(srmsg);

    return SUI_MESSAGE(srmsg);
}

SrnRet sui_add_user(SuiBuffer *buf, const char *nick, UserType type){
    SrnRet ret;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SUI_IS_BUFFER(buf), SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);

    list = sui_buffer_get_user_list(buf);

    ret = srain_user_list_add(list, nick, type);
    if (RET_IS_OK(ret)){
        // FIXME
        // comp = sui_buffer_get_entry_completion(buf);
        // srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return ret;
}

SrnRet sui_rm_user(SuiBuffer *buf, const char *nick){
    SrnRet ret;
    SuiBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SUI_IS_BUFFER(buf), SRN_ERR);
    g_return_val_if_fail(nick, SRN_ERR);

    list = sui_buffer_get_user_list(buf);

    ret = srain_user_list_rm(list, nick);
    if (RET_IS_OK(ret)){
        // FIXME
        // comp = sui_buffer_get_entry_completion(SUI_BUFFER(buf));
        // srain_entry_completion_rm_keyword(comp, nick);
    }

    return ret;
}

SrnRet sui_ren_user(SuiBuffer *buf, const char *old_nick, const char *new_nick,
        UserType type){
    SrnRet ret;
    SuiBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SUI_IS_BUFFER(buf), SRN_ERR);
    g_return_val_if_fail(old_nick, SRN_ERR);
    g_return_val_if_fail(new_nick, SRN_ERR);

    list = sui_buffer_get_user_list(buf);

    /* Your nick changed */
    // FIXME: new-ui
    //
    ret = srain_user_list_rename(list, old_nick, new_nick, type);
    if (RET_IS_OK(ret)){
        // FIXME
        // comp = sui_buffer_get_entry_completion(buf);
        // srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        // srain_entry_completion_rm_keyword(comp, new_nick);
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

void sui_message_append_message(SuiBuffer *buf, SuiMessage *smsg, const char *msg){
    g_return_if_reached();

    SuiBuffer *buffer;

    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(smsg);
    g_return_if_fail(msg);

    // TODO
    // sui_message_append_message(smsg, msg);

    if (SUI_IS_RECV_MESSAGE(smsg)){
        sui_window_side_bar_update(sui_get_cur_window(), buf,
                gtk_label_get_text(SUI_RECV_MESSAGE(smsg)->nick_label), msg);
    } else if (SUI_IS_SEND_MESSAGE(smsg)) {
        sui_window_side_bar_update(sui_get_cur_window(), buf, _("You"), msg);
    } else {
        WARN_FR("Append message is not available for message %p", smsg);
    }
}

void sui_message_mentioned(SuiMessage *smsg){
    g_return_if_reached();
    g_return_if_fail(smsg);

    srain_msg_list_highlight_message(smsg);
}

/**
 * @brief sui_message_noitfy Send a desktop notification, if windows is active,
 *        notification will not be sent.
 *
 * @param smsg
 */
void sui_message_notify(SuiMessage *smsg){
    g_return_if_reached();
    const char *title;
    const char *msg;
    const char *icon;

    g_return_if_fail(smsg);

    if (sui_window_is_active(sui_get_cur_window())){
        return;
    }

    title = NULL;
    msg = sui_message_get_message(smsg);
    icon = "im.srain.Srain";

    if (SUI_IS_RECV_MESSAGE(smsg)){
        // TODO
        title = "TODO";
    }
    else if (SUI_IS_MISC_MESSAGE(smsg)){
        SuiMiscMessage *ssmsg = SUI_MISC_MESSAGE(smsg);
        if (ssmsg->style == SUI_MISC_MESSAGE_STYLE_ERROR){
            title = _("Error");
            icon = "srain-red";
        }
        else if (ssmsg->style == SUI_MISC_MESSAGE_STYLE_ACTION){
            title = _("Action");
        }
    }

    g_return_if_fail(title);

    snotify_notify(title, msg, icon);
    sui_window_tray_icon_stress(sui_get_cur_window(), 1);
}

void sui_add_completion(SuiBuffer *buf, const char *keyword){
    SrainEntryCompletion *comp;

    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(keyword);

    // comp = sui_buffer_get_entry_completion(buffer);

    // srain_entry_completion_add_keyword(comp, keyword, KEYWORD_NORMAL);
}

void sui_rm_completion(SuiBuffer *buf, const char *keyword){
    SuiBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(buf);
    g_return_if_fail(SUI_IS_BUFFER(buf));
    g_return_if_fail(keyword);

    buffer = buf;
    // FIXME
    // comp = sui_buffer_get_entry_completion(buffer);
    // srain_entry_completion_rm_keyword(comp, keyword);
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

}

void sui_chan_list_add(SuiBuffer *buf, const char *chan, int users,
        const char *topic){
    g_return_if_fail(buf);
    g_return_if_fail(chan);
    g_return_if_fail(topic);

    sui_proc_pending_event();
}

void sui_chan_list_end(SuiBuffer *buf){
    g_return_if_fail(buf);
}

void sui_server_list_add(const char *server){
    SrainConnectPopover *popover;

    popover = sui_window_get_connect_popover(sui_get_cur_window());
    srain_connect_popover_add_server(popover, server);
}
