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

/* @file sui.c
 * @brief Srain UI module interfaces
 * @author Shengyu Zhang <srain@srain.im>
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
#include "srain_chat_buffer.h"
#include "theme.h"
#include "snotify.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"
#include "ret.h"

struct _SuiSession{
    SrainBuffer *buffer;

    SuiSessionFlag flag;
    SuiEvents *events;
    SuiPrefs *prefs;

    void *ctx;
};

bool is_app_run = FALSE;
SuiAppEvents *app_events = NULL;
SuiAppPrefs *app_prefs = NULL;

void sui_main_loop(int argc, char *argv[], SuiAppEvents *events, SuiAppPrefs *prefs){
    SrnRet ret;

    g_return_if_fail(events);
    g_return_if_fail(prefs);

    app_events = events;
    app_prefs = prefs;

    ret = sui_app_prefs_check(app_prefs);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
    }

    snotify_init();

    if (theme_load(prefs->theme) == SRN_ERR){
        char *errmsg = g_strdup_printf(_("Failed to load theme \"%s\""), prefs->theme);
        ERR_FR(errmsg);
        sui_message_box(_("Error"), errmsg);
        g_free(errmsg);
    }

    is_app_run = TRUE;
    g_application_run(G_APPLICATION(srain_app_new()), argc, argv);
    is_app_run = FALSE;

    snotify_finalize();
}

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiSession *sui_new_session(SuiEvents *events, SuiPrefs *prefs, SuiSessionFlag flag){
    SrnRet ret;
    SuiSession *sui;

    g_return_val_if_fail(events, NULL);
    g_return_val_if_fail(prefs, NULL);

    ret = sui_prefs_check(prefs);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
        return NULL;
    }

    sui = g_malloc0(sizeof(SuiSession));

    // sui->buffer = NULL; // via g_malloc0()
    sui->flag = flag;
    sui->events = events;
    sui->prefs = prefs;

    return sui;
}

void sui_free_session(SuiSession *sui){
    g_return_if_fail(sui);

    if (sui->buffer){
        srain_window_rm_buffer(srain_win, sui->buffer);
    }
    g_free(sui);
}

int sui_start_session(SuiSession *sui, const char *name, const char *remark){
    SuiSessionFlag flag;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(name, SRN_ERR);
    g_return_val_if_fail(remark, SRN_ERR);

    flag = sui->flag;
    if (flag & SUI_SESSION_SERVER) {
        // type = CHAT_SERVER;
    }
    else if (flag & SUI_SESSION_CHANNEL) {
        // type = CHAT_CHANNEL;
    }
    else if (flag & SUI_SESSION_DIALOG) {
        // type = CHAT_PRIVATE;
    } else {
        ERR_FR("Chat type not found in SuiSessionFlag 0x%x", flag);
        return SRN_ERR;
    }

    sui->buffer = srain_window_add_buffer(srain_win, sui, name, remark);

    if (!sui->buffer){
        return SRN_ERR;
    }

    srain_buffer_show_topic(sui->buffer, sui->prefs->show_topic);
    if (SRAIN_IS_CHAT_BUFFER(sui->buffer)){
        srain_chat_buffer_show_user_list(SRAIN_CHAT_BUFFER(sui->buffer),
                sui->prefs->show_user_list);
    }

    return SRN_OK;
}

void sui_end_session(SuiSession *sui){
    g_return_if_fail(sui);

    srain_window_rm_buffer(srain_win, sui->buffer);
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

SuiPrefs *sui_get_prefs(SuiSession *sui){
    g_return_val_if_fail(sui, NULL);

    return sui->prefs;
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
    g_return_val_if_fail(msg, NULL);

    buffer = sui->buffer;
    g_return_val_if_fail(SRAIN_IS_BUFFER(buffer), NULL);

    list = srain_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_sys_msg_new(msg, type);
    sui_message_set_ctx(smsg, sui);
    srain_msg_list_add_message(list, smsg);

    if (type != SYS_MSG_NORMAL){
        srain_window_stack_sidebar_update(srain_win, buffer, NULL, msg);
    }

    return smsg;
}

SuiMessage *sui_add_sent_msg(SuiSession *sui, const char *msg){
    SrainBuffer *buffer;
    SrainMsgList *list;
    SuiMessage *smsg;

    g_return_val_if_fail(msg, NULL);

    g_return_val_if_fail(sui, NULL);
    buffer = sui->buffer;

    g_return_val_if_fail(SRAIN_IS_BUFFER(buffer), NULL);

    list = srain_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_send_msg_new(msg);
    sui_message_set_ctx(smsg, sui);
    srain_msg_list_add_message(list, smsg);

    srain_window_stack_sidebar_update(srain_win, buffer, _("You"), msg);

    return smsg;
}

SuiMessage *sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id,
        const char *msg){
    SrainBuffer *buffer;
    SrainMsgList *list;
    SrainEntryCompletion *comp;
    SuiMessage *smsg;

    g_return_val_if_fail(sui, NULL);
    g_return_val_if_fail(nick, NULL);
    g_return_val_if_fail(msg, NULL);
    if (!id) id = "";

    buffer = sui->buffer;
    g_return_val_if_fail(SRAIN_IS_BUFFER(buffer), NULL);

    list = srain_buffer_get_msg_list(buffer);
    smsg = (SuiMessage *)srain_recv_msg_new(nick, id, msg);
    sui_message_set_ctx(smsg, sui);
    srain_recv_msg_show_avatar(SRAIN_RECV_MSG(smsg), sui->prefs->show_avatar);
    srain_msg_list_add_message(list, smsg);

    srain_window_stack_sidebar_update(srain_win, buffer, nick, msg);
    if (strlen(id) != 0){
        comp = srain_buffer_get_entry_completion(buffer);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_TMP);
    }

    return smsg;
}

int sui_add_user(SuiSession *sui, const char *nick, UserType type){
    SrnRet ret;
    SrainBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(nick, SRN_ERR);

    g_return_val_if_fail(sui, SRN_ERR);
    buffer = sui->buffer;

    g_return_val_if_fail(SRAIN_IS_BUFFER(buffer), SRN_ERR);

    return SRN_OK;
    /*
    list = srain_buffer_get_user_list(buffer);

    ret = srain_user_list_add(list, nick, type);
    if (RET_IS_OK(ret)){
        comp = srain_buffer_get_entry_completion(buffer);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return ret;
    */
}

int sui_rm_user(SuiSession *sui, const char *nick){
    SrnRet ret;
    SrainBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(nick, SRN_ERR);

    g_return_val_if_fail(sui, SRN_ERR);
    buffer = sui->buffer;

    g_return_val_if_fail(SRAIN_IS_BUFFER(buffer), SRN_ERR);

    return SRN_OK;
    /*
    list = srain_buffer_get_user_list(buffer);

    ret = srain_user_list_rm(list, nick);
    if (RET_IS_OK(ret)){
        comp = srain_buffer_get_entry_completion(buffer);
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return ret;
    */
}

int sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick,
        UserType type){
    SrnRet ret;
    SrainBuffer *buffer;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(old_nick, SRN_ERR);
    g_return_val_if_fail(new_nick, SRN_ERR);

    g_return_val_if_fail(sui, SRN_ERR);
    buffer = sui->buffer;

    g_return_val_if_fail(SRAIN_IS_BUFFER(buffer), SRN_ERR);

    /* Your nick changed */
    if (strcmp(old_nick, srain_buffer_get_nick(buffer)) == 0){
        srain_buffer_set_nick(buffer, new_nick);
    }

    return SRN_OK;

    /*
    list = srain_buffer_get_user_list(buffer);

    ret = srain_user_list_rename(list, old_nick, new_nick, type);
    if (RET_IS_OK(ret)){
        comp = srain_buffer_get_entry_completion(buffer);
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }

    return ret;
    */
}

void sui_set_topic(SuiSession *sui, const char *topic){
    SrainBuffer *buffer;

    g_return_if_fail(sui);
    g_return_if_fail(topic);
    buffer = sui->buffer;

    g_return_if_fail(SRAIN_IS_BUFFER(buffer));

    srain_buffer_set_topic(buffer, topic);
}

void sui_set_topic_setter(SuiSession *sui, const char *setter){
    SrainBuffer *buffer;

    g_return_if_fail(sui);
    g_return_if_fail(setter);
    buffer = sui->buffer;

    g_return_if_fail(SRAIN_IS_BUFFER(buffer));

    // srain_buffer_set_topic_setter(buffer, setter);
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
    g_return_if_fail(smsg);
    g_return_if_fail(msg);

    buffer = sui->buffer;
    g_return_if_fail(SRAIN_IS_BUFFER(buffer));

    srain_msg_append_msg(smsg, msg);

    if (SRAIN_IS_RECV_MSG(smsg)){
        srain_window_stack_sidebar_update(srain_win, buffer,
                gtk_label_get_text(SRAIN_RECV_MSG(smsg)->nick_label), msg);
    }
    else if (SRAIN_IS_SEND_MSG(smsg)) {
        srain_window_stack_sidebar_update(srain_win, buffer, _("You"), msg);
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

    if (sui->prefs->preview_image){
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

    if (srain_window_is_active(srain_win)){
        return;
    }

    sui = sui_message_get_ctx(smsg);
    if (sui && !sui->prefs->notify){
        return;
    }

    title = NULL;
    msg = gtk_label_get_text(smsg->msg_label);
    icon = "srain";

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
    srain_window_tray_icon_stress(srain_win, 1);
}

void sui_add_completion(SuiSession *sui, const char *keyword){
    SrainBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(sui);
    g_return_if_fail(keyword);

    buffer = sui->buffer;
    g_return_if_fail(SRAIN_IS_BUFFER(buffer));

    comp = srain_buffer_get_entry_completion(buffer);

    srain_entry_completion_add_keyword(comp, keyword, KEYWORD_NORMAL);
}

void sui_rm_completion(SuiSession *sui, const char *keyword){
    SrainBuffer *buffer;
    SrainEntryCompletion *comp;

    g_return_if_fail(sui);
    g_return_if_fail(keyword);

    buffer = sui->buffer;
    g_return_if_fail(SRAIN_IS_BUFFER(buffer));

    comp = srain_buffer_get_entry_completion(buffer);

    srain_entry_completion_rm_keyword(comp, keyword);
}

void sui_message_box(const char *title, const char *msg){
    GtkMessageDialog *dia;
    char *markuped_msg;

    if (!is_app_run){
        gtk_init(0, NULL);
    }

    dia = GTK_MESSAGE_DIALOG(
            gtk_message_dialog_new(GTK_WINDOW(srain_win),
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
    SrainJoinPopover *popover;

    g_return_if_fail(sui);

    popover = srain_window_get_join_popover(srain_win);
    srain_join_popover_start_chan(popover);
}

void sui_chan_list_add(SuiSession *sui, const char *chan, int users,
        const char *topic){
    SrainJoinPopover *popover;

    g_return_if_fail(sui);
    g_return_if_fail(chan);
    g_return_if_fail(topic);

    popover = srain_window_get_join_popover(srain_win);
    srain_join_popover_add_chan(popover, chan, users, topic);
    sui_proc_pending_event();
}

void sui_chan_list_end(SuiSession *sui){
    SrainJoinPopover *popover;

    g_return_if_fail(sui);

    popover = srain_window_get_join_popover(srain_win);
    srain_join_popover_end_chan(popover);
}

void sui_server_list_add(const char *server){
    SrainConnectPopover *popover;

    popover = srain_window_get_connect_popover(srain_win);
    srain_connect_popover_add_server(popover, server);
}
