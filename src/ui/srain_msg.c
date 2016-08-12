/**
 * @file srain_msg.c
 * @brief GtkWidget subclass used to display message
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <time.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "ui.h"
#include "ui_common.h"
#include "ui_hdr.h"
#include "nick_menu.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_msg.h"
#include "srain_image.h"

#include "markup.h"
#include "download.h"
#include "log.h"
#include "get_path.h"
#include "i18n.h"
#include "meta.h"

/**
 * @brief Get the selected text (utf-8 supported) of `label`,
 *      if no text was selected, return all of the text in this label,
 *      if there is any '\n'(newline) in the text, strip it.
 *
 * @return A allocated (char *), it should be freed by `free()`
 */
static char* label_get_selection(GtkLabel *label){
    int start, end;
    const char *msg;
    char *sel_msg;
    if (!label) return NULL;

    msg = gtk_label_get_text(label);

    if (gtk_label_get_selection_bounds(label, &start, &end)){
        sel_msg = g_utf8_substring(msg, start, end);
    } else {
        sel_msg = strdup(msg);
    }

    return sel_msg;
}

static void copy_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char* sel_msg;
    char* line;
    GString *str;
    GtkClipboard *cb;
    SrainRecvMsg *smsg;

    smsg = SRAIN_RECV_MSG(user_data);

    if ((sel_msg = label_get_selection(smsg->msg_label)) == NULL) return;

    /* Get the clipboard object */
    cb = gtk_widget_get_clipboard(GTK_WIDGET(smsg), GDK_SELECTION_CLIPBOARD);

    str = g_string_new("");
    line = strtok(sel_msg, "\n");
    while (line){
        g_string_append_printf(str, "[%s] <%s> %s\n",
                gtk_label_get_text(smsg->time_label),
                gtk_label_get_text(smsg->nick_label),
                line);
        line = strtok(NULL, "\n");
    }
    /* Set clipboard text */
    gtk_clipboard_set_text(cb, str->str, -1);

    g_string_free(str, TRUE);
    g_free(sel_msg);
}

static void froward_submenu_item_on_activate(GtkWidget* widget, gpointer user_data){
    char *sel_msg;
    char *line;
    const char *srv_name;
    GString *str;
    SrainChan *chan;
    SrainRecvMsg *smsg;

    smsg = SRAIN_RECV_MSG(user_data);
    if ((sel_msg = label_get_selection(smsg->msg_label)) == NULL) return;

    srv_name = srain_chan_get_srv_name(srain_window_get_cur_chan(srain_win));
    chan = srain_window_get_chan_by_name(srain_win, srv_name,
            gtk_menu_item_get_label(GTK_MENU_ITEM(widget)));

    line = strtok(sel_msg, "\n");
    while (line){
        str = g_string_new("");
        g_string_printf(str, _("%s <fwd %s@%s>"), line,
                gtk_label_get_text(smsg->nick_label),
                srain_chan_get_chan_name(srain_window_get_cur_chan(srain_win)));
        line = strtok(NULL, "\n");

        ui_send_msg_sync(
                srain_chan_get_srv_name(chan),
                srain_chan_get_chan_name(chan),
                str->str);
        if (ui_hdr_srv_send(chan, str->str) < 0){
            ui_sys_msg_sync(
                    srain_chan_get_srv_name(chan),
                    srain_chan_get_chan_name(chan),
                    _("Failed to send message"),
                    SYS_MSG_ERROR);
        }
        g_string_free(str, TRUE);
    }

    g_free(sel_msg);
}

static void msg_label_on_popup(GtkLabel *label, GtkMenu *menu,
        gpointer user_data){
    GtkMenuItem *copy_menu_item;
    GtkMenuItem *forward_menu_item;
    SrainRecvMsg *smsg;

    smsg = SRAIN_RECV_MSG(user_data);

    /* Create menuitem copy_menu_item */
    copy_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Copy message")));
    gtk_widget_show(GTK_WIDGET(copy_menu_item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(copy_menu_item));
    g_signal_connect(copy_menu_item, "activate",
                G_CALLBACK(copy_menu_item_on_activate), smsg);

    /* Create menuitem forward_menu_item */
    forward_menu_item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(_("Forward to...")));
    gtk_widget_show(GTK_WIDGET(forward_menu_item));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(forward_menu_item));

    /* Create submenu of forward_menu_item */
    GList *chans;
    GtkMenuItem *item;
    GtkMenu *forward_submenu = GTK_MENU(gtk_menu_new());
    SrainChan *chan;

    chan = srain_window_get_cur_chan(srain_win);

    chans = srain_window_get_chans_by_srv_name(srain_win,
            srain_chan_get_srv_name(chan));
    if (!chans) return;
    /* Skip META_SERVER */
    chans = g_list_next(chans);

    gtk_menu_item_set_submenu(forward_menu_item, GTK_WIDGET(forward_submenu));

    while (chans){
        item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                    srain_chan_get_chan_name(chans->data)));
        gtk_widget_show(GTK_WIDGET(item));
        g_signal_connect(item, "activate",
                G_CALLBACK(froward_submenu_item_on_activate), smsg);
        gtk_menu_shell_append(GTK_MENU_SHELL(forward_submenu), GTK_WIDGET(item));

        chans = g_list_next(chans);
    }

    g_list_free(chans);
}

static gboolean nick_button_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    GtkLabel *nick_label;

    nick_label = GTK_LABEL(user_data);
    if (event->button == 3){
        nick_menu_popup(event, gtk_label_get_text(nick_label));

        return TRUE;
    }
    return FALSE;
}

static void nick_button_on_click(GtkWidget *widget, gpointer *user_data){
    GString *str;
    GtkLabel *nick_label;

    nick_label = GTK_LABEL(user_data);
    str = g_string_new(gtk_label_get_text(nick_label));
    str = g_string_append(str, ": ");

    srain_chan_insert_text(srain_window_get_cur_chan(srain_win), str->str, 0);
    g_string_free(str, TRUE);
}

/* ================ SRAIN_SYS_MSG ================ */
G_DEFINE_TYPE(SrainSysMsg, srain_sys_msg, GTK_TYPE_BOX);

static void srain_sys_msg_init(SrainSysMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_sys_msg_class_init(SrainSysMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/sys_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSysMsg, msg_label);
}

SrainSysMsg* srain_sys_msg_new(const char *msg, SysMsgType type){
    SrainSysMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SYS_MSG, NULL);
    switch (type){
        case SYS_MSG_NORMAL:
            gtk_widget_set_name(GTK_WIDGET(smsg), "normal_sys_msg_box");
            break;
        case SYS_MSG_ERROR:
            gtk_widget_set_name(GTK_WIDGET(smsg), "error_sys_msg_box");
            break;
        case SYS_MSG_NOTICE:
            gtk_widget_set_name(GTK_WIDGET(smsg), "notice_sys_msg_box");
            break;
        case SYS_MSG_ACTION:
            gtk_widget_set_name(GTK_WIDGET(smsg), "action_sys_msg_box");
            break;
        default:
            ERR_FR("unkown SysMsgType");
    }
    gtk_label_set_text(smsg->msg_label, msg);

    return smsg;
}

/* ================ SRAIN_SEND_MSG ================ */
G_DEFINE_TYPE(SrainSendMsg, srain_send_msg, GTK_TYPE_BOX);

static void srain_send_msg_init(SrainSendMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_send_msg_class_init(SrainSendMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/send_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, padding_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, time_label);
}

SrainSendMsg* srain_send_msg_new(const char *msg){
    char timestr[32];
    GString *img_url;
    GString *markuped_msg;
    SrainSendMsg *smsg;
    SrainImage *simg;

    smsg = g_object_new(SRAIN_TYPE_SEND_MSG, NULL);

    get_cur_time(timestr);
    gtk_label_set_text(smsg->time_label, timestr);

    markuped_msg = markup(msg, &img_url);
    if (markuped_msg){
        gtk_label_set_markup(smsg->msg_label, markuped_msg->str);
        g_string_free(markuped_msg, TRUE);
    } else {
        gtk_label_set_text(smsg->msg_label, msg);
    }

    if (img_url){
        simg = srain_image_new();
        srain_image_set_from_url_async(simg, img_url->str, 300,
                SRAIN_IMAGE_ENLARGE | SRAIN_IMAGE_SPININER | SRAIN_IMAGE_AUTOLOAD);
        g_string_free(img_url, TRUE);
        gtk_container_add(GTK_CONTAINER(smsg->padding_box), GTK_WIDGET(simg));
        gtk_container_set_border_width(GTK_CONTAINER(simg), 6);
        gtk_widget_show(GTK_WIDGET(simg));
    }

    return smsg;
}

/* ================ SRAIN_RECV_MSG ================ */
G_DEFINE_TYPE(SrainRecvMsg, srain_recv_msg, GTK_TYPE_BOX);

static void srain_recv_msg_init(SrainRecvMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(self->nick_button, "clicked",
            G_CALLBACK(nick_button_on_click), self->nick_label);
    g_signal_connect(self->nick_button, "button-press-event",
            G_CALLBACK(nick_button_on_popup), self->nick_label);
    g_signal_connect(self->msg_label, "populate-popup",
            G_CALLBACK(msg_label_on_popup), self);

}

static void srain_recv_msg_class_init(SrainRecvMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/recv_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, avatar_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, padding_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, time_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, nick_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, identify_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, nick_button);
}

SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg){
    char timestr[32];
    char *avatar_path;
    GString *markuped_msg;
    GString *img_url;
    SrainImage *simg;
    SrainImage *avatar_simg;
    SrainRecvMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_RECV_MSG, NULL);

    get_cur_time(timestr);
    gtk_label_set_text(smsg->time_label, timestr);
    gtk_label_set_text(smsg->nick_label, nick);
    gtk_label_set_text(smsg->identify_label, id);

    img_url = NULL;
    markuped_msg = markup(msg, &img_url);
    if (markuped_msg){
        gtk_label_set_markup(smsg->msg_label, markuped_msg->str);
        g_string_free(markuped_msg, TRUE);
    } else {
        gtk_label_set_text(smsg->msg_label, msg);
    }

    /* Image in message */
    if (img_url){
        simg = srain_image_new();
        srain_image_set_from_url_async(simg, img_url->str, 300,
                SRAIN_IMAGE_ENLARGE | SRAIN_IMAGE_SPININER | SRAIN_IMAGE_AUTOLOAD);
        g_string_free(img_url, TRUE);
        gtk_container_add(GTK_CONTAINER(smsg->padding_box), GTK_WIDGET(simg));
        gtk_container_set_border_width(GTK_CONTAINER(simg), 6);
        gtk_widget_show(GTK_WIDGET(simg));
    }

    avatar_simg = srain_image_new();
    avatar_path = get_avatar_path(nick);

    if (!avatar_path) avatar_path = get_pixmap_path("srain-avatar.png");
    if (avatar_path){
        srain_image_set_from_file(avatar_simg, avatar_path, 36, SRAIN_IMAGE_AUTOLOAD);
        g_free(avatar_path);
    }

    gtk_container_add(GTK_CONTAINER(smsg->avatar_box), GTK_WIDGET(avatar_simg));
    gtk_widget_show(GTK_WIDGET(avatar_simg));

    return smsg;
}
