/**
 * @file srain_msg.c
 * @brief GtkWidget subclass used to display message
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <time.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>

#include "ui_common.h"
#include "ui_intf.h"
#include "srain_window.h"
#include "srain_msg.h"
#include "srain_image.h"

#include "markup.h"
#include "download.h"
#include "log.h"
#include "plugin.h"
#include "get_path.h"

static void nick_button_on_click(GtkWidget *widget, gpointer *user_data){
    GString *cmd;

    cmd = g_string_new(NULL);

    g_string_printf(cmd, "/whois %s", (char *)user_data);
    ui_intf_server_cmd(NULL, cmd->str);

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
        case SYS_MSG_COMMAND:
            gtk_widget_set_name(GTK_WIDGET(smsg), "command_sys_msg_box");
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

    /* avatar in message */
    if (img_url){
        simg = srain_image_new();
        srain_image_set_from_url_async(simg, img_url->str, 300,
                SRAIN_IMAGE_ENLARGE | SRAIN_IMAGE_SPININER | SRAIN_IMAGE_AUTOLOAD);
        g_string_free(img_url, TRUE);
        gtk_container_add(GTK_CONTAINER(smsg->padding_box), GTK_WIDGET(simg));
        gtk_container_set_border_width(GTK_CONTAINER(simg), 6);
        gtk_widget_show(GTK_WIDGET(simg));
    }

    /* avatar TODO */
    avatar_path = plugin_avatar(nick, "", "");

    avatar_simg = srain_image_new();
    avatar_path =  get_pixmap_path("srain-avatar.png");
    if (avatar_path){
        srain_image_set_from_file(avatar_simg, avatar_path, 36, SRAIN_IMAGE_AUTOLOAD);
        g_free(avatar_path);
    }
    srain_image_set_from_url_async(avatar_simg, avatar_path, 36, SRAIN_IMAGE_AUTOLOAD);
    gtk_container_add(GTK_CONTAINER(smsg->avatar_box), GTK_WIDGET(avatar_simg));
    gtk_widget_show(GTK_WIDGET(avatar_simg));

    if (strlen(gtk_label_get_text(smsg->identify_label)) != 0){
        g_signal_connect(smsg->nick_button, "clicked",
                G_CALLBACK(nick_button_on_click), (char *)gtk_label_get_text(smsg->identify_label));
    } else {
        g_signal_connect(smsg->nick_button, "clicked",
                G_CALLBACK(nick_button_on_click), (char *)gtk_label_get_text(smsg->nick_label));
    }

    return smsg;
}
