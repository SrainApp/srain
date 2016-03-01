/**
 * @file srain_msg.c
 * @brief GtkWidget subclass used to display message
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include "ui_common.h"
#include "srain_window.h"
#include "srain_msg.h"
#include "srain_detail_dialog.h"
#include "srain_image_window.h"
#include "log.h"

/* display bigger image */
static void image_on_click(gpointer *user_data , GdkEventButton *event){
    char *path;
    SrainImageWindow *win;

    if (event->button == 1){
        path = (char *)user_data;
        win = srain_image_window_new(path);
        gtk_window_present(GTK_WINDOW(win));
    }
}

static void nick_on_click(GtkWidget *widget, gpointer *user_data){
    char *nick;
    SrainWindow* toplevel;
    SrainDetailDialog *dlg;

    nick = (char *)user_data;
    toplevel = SRAIN_WINDOW(gtk_widget_get_toplevel(widget));
    dlg = srain_detail_dialog_new(toplevel, nick, "");
    gtk_window_present(GTK_WINDOW(dlg));
}

static gint menu_popup(GtkWidget *label, GdkEventButton *event, GtkWidget *menu){
    if (event->button == 3 && !gtk_label_get_selection_bounds(GTK_LABEL(label), NULL, NULL)){
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
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

SrainSysMsg* srain_sys_msg_new(const char *msg){
    SrainSysMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SYS_MSG, NULL);
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
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, time_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, image_eventbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, image);
}

SrainSendMsg* srain_send_msg_new(const char *msg, const char *img_path){
    char timestr[32];
    SrainSendMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SEND_MSG, NULL);

    get_cur_time(timestr);
    gtk_label_set_text(smsg->time_label, timestr);
    gtk_label_set_text(smsg->msg_label, msg);
    if (img_path){
        g_signal_connect_swapped(smsg->image_eventbox, "button_release_event",
                G_CALLBACK(image_on_click), (char *)img_path);
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size((char *)img_path, 300, 300, NULL);
        gtk_image_set_from_pixbuf(smsg->image, pixbuf);
        g_object_unref (pixbuf);
    }

    return smsg;
}

/* ================ SRAIN_RECV_MSG ================ */
G_DEFINE_TYPE(SrainRecvMsg, srain_recv_msg, GTK_TYPE_BOX);

static void srain_recv_msg_init(SrainRecvMsg *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_recv_msg_class_init(SrainRecvMsgClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/recv_msg.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, time_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, image);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, image_eventbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, avatar_image);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, nick_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, identify_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainRecvMsg, nick_button);
}

SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg, const char *img_path){
    char timestr[32];
    SrainRecvMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_RECV_MSG, NULL);

    get_cur_time(timestr);
    gtk_label_set_text(smsg->time_label, timestr);
    gtk_label_set_text(smsg->msg_label, msg);
    gtk_label_set_text(smsg->nick_label, nick);
    gtk_label_set_text(smsg->identify_label, id);
    g_signal_connect(smsg->nick_button, "clicked", G_CALLBACK(nick_on_click),
            (char *)gtk_label_get_text(smsg->nick_label));
    if (img_path){
        g_signal_connect_swapped(smsg->image_eventbox, "button_release_event",
                G_CALLBACK(image_on_click), (char *)img_path);
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size((char *)img_path, 300, 300, NULL);
        gtk_image_set_from_pixbuf(smsg->image, pixbuf);
        g_object_unref (pixbuf);
    }

    return smsg;
}
