#include <gtk/gtk.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "ui.h"
#include "ui_common.h"
#include "srain_msg.h"
#include "log.h"

static void get_cur_time(char *timestr){
    time_t curtime;

    time(&curtime);
    strftime(timestr, 32, "%m-%d %H:%M", localtime(&curtime));
    timestr[31] = '\0';
}

/* display bigger image */
static void image_on_click(char *path, GdkEventButton *event){
    if (event->button == 1){
        image_window_init(path);
    }
}

static void nick_on_click(char *nick){
    detail_dialog_init(nick, "");
}

static gint menu_popup(GtkWidget *label, GdkEventButton *event, GtkWidget *menu){
    if (event->button == 3 && !gtk_label_get_selection_bounds(GTK_LABEL(label), NULL, NULL)){
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

/* ================ SRAIN_SYS_MSG ================ */
struct _SrainSysMsg {
    GtkBox parent;
    GtkLabel *msg_label;
};

struct _SrainSysMsgClass {
    GtkBoxClass parent_class;
};

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
struct _SrainSendMsg {
    GtkBox parent;
    GtkLabel *msg_label;
    GtkLabel *time_label;
    GtkEventBox *image_eventbox;
    GtkImage *image;
};

struct _SrainSendMsgClass {
    GtkBoxClass parent_class;
};

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

SrainSendMsg* srain_send_msg_new(const char *msg){
    char timestr[32];
    SrainSendMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_SEND_MSG, NULL);

    get_cur_time(timestr);
    gtk_label_set_text(smsg->time_label, timestr);
    gtk_label_set_text(smsg->msg_label, msg);
    /*
    g_signal_connect_swapped(smsg->image_eventbox, "button_release_event",
            G_CALLBACK(image_on_click), (char *)"./img.png");

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size ("./img.png", 300, 300, NULL);
    gtk_image_set_from_pixbuf(smsg->image, pixbuf);
    g_object_unref (pixbuf);
    */

    return smsg;
}

/* ================ SRAIN_RECV_MSG ================ */
struct _SrainRecvMsg {
    GtkBox parent;
    GtkLabel *msg_label;
    GtkLabel *time_label;
    GtkImage *image;
    GtkEventBox *image_eventbox;
    GtkImage *avatar_image;
    GtkLabel *nick_label;
    GtkLabel *identify_label;
    GtkButton *nick_button;
};

struct _SrainRecvMsgClass {
    GtkBoxClass parent_class;
};

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

SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg){
    char timestr[32];
    SrainRecvMsg *smsg;

    smsg = g_object_new(SRAIN_TYPE_RECV_MSG, NULL);

    get_cur_time(timestr);
    gtk_label_set_text(smsg->time_label, timestr);
    gtk_label_set_text(smsg->msg_label, msg);
    gtk_label_set_text(smsg->nick_label, nick);
    gtk_label_set_text(smsg->identify_label, id);
    g_signal_connect_swapped(smsg->nick_button, "clicked", G_CALLBACK(nick_on_click), (char *)nick);
    /*
    g_signal_connect_swapped(smsg->image_eventbox, "button_release_event",
            G_CALLBACK(image_on_click), (char *)"./img.png");

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size ("./img.png", 300, 300, NULL);
    gtk_image_set_from_pixbuf(smsg->image, pixbuf);
    g_object_unref (pixbuf);
    */
    return smsg;
}
