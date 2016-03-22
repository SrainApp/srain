/**
 * @file srain_msg.c
 * @brief GtkWidget subclass used to display message
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include "ui.h"
#include "ui_common.h"
#include "srain_window.h"
#include "srain_msg.h"
#include "srain.h"
#include "srain_image_window.h"
#include "markup.h"
#include "download.h"
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

static void nick_button_on_click(GtkWidget *widget, gpointer *user_data){
    GString *cmd;

    cmd = g_string_new(NULL);
    g_string_printf(cmd, "/whois %s", (char *)user_data);
    srain_cmd(NULL, cmd->str);
    g_string_free(cmd, TRUE);
}

static gint menu_popup(GtkWidget *label, GdkEventButton *event, GtkWidget *menu){
    if (event->button == 3
            && !gtk_label_get_selection_bounds(GTK_LABEL(label), NULL, NULL)){
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                event->button, event->time);
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

SrainSysMsg* srain_sys_msg_new(sys_msg_type_t type, const char *msg){
    SrainSysMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SYS_MSG, NULL);
    switch (type){
        case SYS_MSG_NORMAL:
            gtk_widget_set_name(GTK_WIDGET(smsg), "normal_sys_msg_box");
            break;
        case SYS_MSG_ERROR:
            gtk_widget_set_name(GTK_WIDGET(smsg), "error_sys_msg_box");
            break;
        case SYS_MSG_COMMAND:
            gtk_widget_set_name(GTK_WIDGET(smsg), "command_sys_msg_box");
            break;
        case SYS_MSG_ACTION:
            gtk_widget_set_name(GTK_WIDGET(smsg), "action_sys_msg_box");
            break;
        default:
            ERR_FR("unkown sys_msg_type_t");
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
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, msg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, time_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, image_eventbox);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainSendMsg, image);
}

static gboolean srain_send_msg_set_image(SrainSendMsg *msg){
    GdkPixbuf *pixbuf;

    LOG_FR("%s", msg->image_path->str);

    if (msg->image_path){
        g_signal_connect_swapped(msg->image_eventbox,
                                 "button_release_event",
                                 G_CALLBACK(image_on_click),
                                 msg->image_path->str);
        pixbuf = gdk_pixbuf_new_from_file_at_size(msg->image_path->str, 300, 300, NULL);
        gtk_image_set_from_pixbuf(msg->image, pixbuf);
        g_object_unref(pixbuf);
    }

    return FALSE;
}

static void srain_send_msg_set_image_async(SrainSendMsg *msg){
    GString *path;

    LOG_FR("%s", msg->image_path->str);

    path = download(msg->image_path->str);
    if (path){
        g_string_free(msg->image_path, TRUE);
        msg->image_path = path;
        gdk_threads_add_idle((GSourceFunc)srain_send_msg_set_image, msg);
    }
}

SrainSendMsg* srain_send_msg_new(const char *msg){
    char timestr[32];
    GString *img_url;
    GString *markuped_msg;
    SrainSendMsg *smsg;

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
        smsg->image_path = img_url;
        g_thread_new(NULL, (GThreadFunc)srain_send_msg_set_image_async, smsg);
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

static gboolean srain_recv_msg_set_image(SrainRecvMsg *msg){
    GdkPixbuf *pixbuf;

    LOG_FR("%s", msg->image_path->str);

    if (msg->image_path){
        g_signal_connect_swapped(msg->image_eventbox, "button_release_event",
                G_CALLBACK(image_on_click), msg->image_path->str);
        pixbuf = gdk_pixbuf_new_from_file_at_size(msg->image_path->str, 300, 300, NULL);
        gtk_image_set_from_pixbuf(msg->image, pixbuf);
        g_object_unref(pixbuf);
    }

    return FALSE;
}

static void srain_recv_msg_set_image_async(SrainRecvMsg *msg){
    GString *path;

    LOG_FR("%s", msg->image_path->str);

    path = download(msg->image_path->str);
    if (path){
        g_string_free(msg->image_path, TRUE);
        msg->image_path = path;
        gdk_threads_add_idle((GSourceFunc)srain_recv_msg_set_image, msg);
    }
}

SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg){
    char timestr[32];
    GString *markuped_msg;
    GString *img_url;
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

    if (img_url){
        smsg->image_path = img_url;
        g_thread_new(NULL, (GThreadFunc)srain_recv_msg_set_image_async, smsg);
    }

    g_signal_connect(smsg->nick_button, "clicked",
            G_CALLBACK(nick_button_on_click), (char *)gtk_label_get_text(smsg->nick_label));

    return smsg;
}
