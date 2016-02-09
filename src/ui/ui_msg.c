#include <gtk/gtk.h>
#include <assert.h>
#include <string.h>
#include "ui_common.h"
#include "msg.h"
#include "log.h"

/* extern variable from ui_window.c */
extern GtkWidget *chat_panel_stack;

static GtkWidget *msg_bubble_menu;

/* display bigger image */
static void image_on_click(gchar *path, GdkEventButton *event){
    if (event->button == 1){
        image_window_init(path);
    }
}

static void nick_button_on_click(GtkWidget *widget, GdkEventButton *event, GtkLabel *label){
    const gchar *nick;

    nick = gtk_label_get_text(label);
    if (event->button == 1){
        detail_dialog_init(nick, "");
        // replace it with a WHOIS function
    }
}

/* return FALSE when some text in labet are selected, giving up this signal,
 * and default callback function will be called
 */
static gint menu_popup(GtkWidget *label, GdkEventButton *event, GtkWidget *menu){
    if (event->button == 3 && !gtk_label_get_selection_bounds(GTK_LABEL(label), NULL, NULL)){
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }
    return FALSE;
}

void ui_msg_init(){
    GtkBuilder *builder;
    GtkWidget *menu;

    builder = gtk_builder_new_from_file( "../data/ui/msg_bubble.glade");
    UI_BUILDER_GET_WIDGET(builder, msg_bubble_menu);
    /* NOTE: if we do not ref msg_bubble_menu to menu,
     * msg_bubble_menu will be free after g_object_unref(builder)
     * issuse #2
     */
    menu = g_object_ref(msg_bubble_menu);
    g_object_unref(builder);
}

int ui_msg_send(const MsgSend msg){
    GtkBuilder *builder;
    GtkWidget *send_msg_bubble_box;
    GtkWidget *send_msg_label;
    GtkWidget *send_image;
    GtkWidget *send_image_eventbox;
    GtkWidget *send_time_label;
    GtkWidget *chat_msg_listbox;
    GtkWidget *chat_panel_box;

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), msg.chan);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s not found", msg.chan);
        return -1;
    }

    chat_msg_listbox = get_widget_by_name(chat_panel_box, "chat_msg_listbox");
    assert(chat_msg_listbox);
    builder = gtk_builder_new_from_file( "../data/ui/msg_bubble.glade");
    if (msg.img){
        UI_BUILDER_GET_WIDGET(builder, send_image);
        UI_BUILDER_GET_WIDGET(builder, send_image_eventbox);
    }
    UI_BUILDER_GET_WIDGET(builder, send_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, send_msg_label);
    UI_BUILDER_GET_WIDGET(builder, send_time_label);
    if (msg.img){
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (msg.img, 300, 300, NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(send_image), pixbuf);
        g_object_unref (pixbuf);
        g_signal_connect_swapped(send_image_eventbox, "button_release_event", G_CALLBACK(image_on_click), msg.img);
    }

    gtk_label_set_text(GTK_LABEL(send_msg_label), msg.msg);
    gtk_label_set_text(GTK_LABEL(send_time_label), msg.time);

    g_signal_connect(send_msg_label, "button_press_event", G_CALLBACK(menu_popup), msg_bubble_menu);

    gtk_container_add(GTK_CONTAINER(chat_msg_listbox), send_msg_bubble_box);

    g_object_unref(G_OBJECT(builder));

    return 0;
}

gboolean ui_msg_recv(MsgRecv *msg){
    GtkBuilder *builder;
    GtkWidget *recv_msg_bubble_box;
    GtkWidget *avatar_image;
    GtkWidget *recv_image;
    GtkWidget *recv_image_eventbox;
    GtkWidget *nick_label;
    GtkWidget *nick_button;
    GtkWidget *identify_label;
    GtkWidget *recv_msg_label;
    GtkWidget *recv_time_label;
    GtkWidget *chat_msg_listbox;
    GtkWidget *chat_panel_box;

    LOG_FR("called");

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), msg->chan);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s not found", msg->chan);
        return FALSE;
    }

    chat_msg_listbox = get_widget_by_name(chat_panel_box, "chat_msg_listbox");
    assert(chat_msg_listbox);
    builder = gtk_builder_new_from_file( "../data/ui/msg_bubble.glade");
    if (msg->avatar) UI_BUILDER_GET_WIDGET(builder, avatar_image);
    if (msg->img){
        UI_BUILDER_GET_WIDGET(builder, recv_image);
        UI_BUILDER_GET_WIDGET(builder, recv_image_eventbox);
    }
    UI_BUILDER_GET_WIDGET(builder, recv_msg_bubble_box);
    UI_BUILDER_GET_WIDGET(builder, nick_label);
    UI_BUILDER_GET_WIDGET(builder, nick_button);
    UI_BUILDER_GET_WIDGET(builder, identify_label);
    UI_BUILDER_GET_WIDGET(builder, recv_msg_label);
    UI_BUILDER_GET_WIDGET(builder, recv_time_label);

    gtk_label_set_text(GTK_LABEL(nick_label), msg->nick);
    gtk_label_set_text(GTK_LABEL(identify_label), msg->id);
    gtk_label_set_text(GTK_LABEL(recv_time_label), msg->time);
    gtk_label_set_text(GTK_LABEL(recv_msg_label), msg->msg);
    if (msg->img){
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (msg->img, 300, 300, NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(recv_image), pixbuf);
        g_object_unref (pixbuf);
        g_signal_connect_swapped(recv_image_eventbox, "button_release_event", G_CALLBACK(image_on_click), msg->img);
    }

    g_signal_connect(nick_button, "button_release_event", G_CALLBACK(nick_button_on_click), nick_label);
    g_signal_connect(recv_msg_label, "button_press_event", G_CALLBACK(menu_popup), G_OBJECT(msg_bubble_menu));

    gtk_container_add(GTK_CONTAINER(chat_msg_listbox), recv_msg_bubble_box);

    g_object_unref(G_OBJECT(builder));

    return FALSE;
}

int ui_msg_sys(const MsgSys *msg){
    GtkBuilder *builder;
    GtkWidget *sys_msg_label;
    GtkWidget *chat_msg_listbox;
    GtkWidget *chat_panel_box;

    chat_panel_box = gtk_stack_get_child_by_name(GTK_STACK(chat_panel_stack), msg->chan);
    if (!chat_panel_box){
        ERR_FR("chat_panel %s not found", msg->chan);
        return -1;
    }
    chat_msg_listbox = get_widget_by_name(chat_panel_box, "chat_msg_listbox");
    assert(chat_msg_listbox);

    builder = gtk_builder_new_from_file("../data/ui/msg_bubble.glade");
    UI_BUILDER_GET_WIDGET(builder, sys_msg_label);

    gtk_label_set_text(GTK_LABEL(sys_msg_label), msg->msg);

    gtk_container_add(GTK_CONTAINER(chat_msg_listbox), sys_msg_label);

    g_object_unref(G_OBJECT(builder));

    return 0;
}
