#include <gtk/gtk.h>
#include <string.h>

#include "srain_app.h"
#include "srain_window.h"
#include "srain_chat.h"

#include "log.h"

static void nick_menu_item_on_activate(GtkWidget* widget, gpointer user_data){
    const char *nick;
    SrainChat *chat;

    chat = srain_window_get_cur_chat(srain_win);
    nick = user_data;

    if (strcmp(gtk_widget_get_name(widget), "whois_menu_item") == 0){
        // ui_hdr_srv_whois(srain_chat_get_srv_name(chat), nick);
    }
    else if (strcmp(gtk_widget_get_name(widget), "ignore_menu_item") == 0){
        // TODO
        GString *cmd = g_string_new("");

        g_string_printf(cmd, "/ignore %s", nick);
        /*ui_hdr_srv_cmd(
                srain_chat_get_srv_name(chat),
                srain_chat_get_chat_name(chat),
                cmd->str, 0);
                */
        g_string_free(cmd, TRUE);
    }
    else if (strcmp(gtk_widget_get_name(widget), "kick_menu_item") == 0){
        /* ui_hdr_srv_kick(
                srain_chat_get_srv_name(chat),
                srain_chat_get_chat_name(chat),
                nick, "");
                */
    }
    else if (strcmp(gtk_widget_get_name(widget), "chat_menu_item") == 0){
        // ui_hdr_srv_query( srain_chat_get_srv_name(chat), nick);
    }
    else if (strcmp(gtk_widget_get_name(widget), "invite_submenu_item") == 0){
        /* ui_hdr_srv_invite(
                srain_chat_get_srv_name(chat),
                srain_chat_get_chat_name(chat),
                nick); */
    }
    else {
        ERR_FR("Unknown menu item: %s", gtk_widget_get_name(widget));
    }
}

void nick_menu_popup(GdkEventButton *event, const char *nick){
    GList *chats;
    GtkBuilder *builder;
    GtkMenu *nick_menu;
    GtkMenuItem *item;
    GtkMenuItem *whois_menu_item;
    GtkMenuItem *ignore_menu_item;
    GtkMenuItem *kick_menu_item;
    GtkMenuItem *chat_menu_item;
    GtkMenuItem *invite_menu_item;
    SrainChat *chat;

    builder = gtk_builder_new_from_resource ("/org/gtk/srain/nick_menu.glade");

    nick_menu = (GtkMenu *)gtk_builder_get_object(builder, "nick_menu");
    whois_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "whois_menu_item");
    ignore_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "ignore_menu_item");
    kick_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "kick_menu_item");
    chat_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "chat_menu_item");
    invite_menu_item = (GtkMenuItem *)gtk_builder_get_object(builder, "invite_menu_item");

    g_signal_connect(whois_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
    g_signal_connect(ignore_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
    g_signal_connect(kick_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
    g_signal_connect(chat_menu_item, "activate",
            G_CALLBACK(nick_menu_item_on_activate), (char *)nick);

    /******************************************/
    chat = srain_window_get_cur_chat(srain_win);
    chats = srain_window_get_chats_by_srv_name(srain_win,
            srain_chat_get_srv_name(chat));

    /* Skip META_SERVER */
    chats = g_list_next(chats);

    /* Create subitem of invite_menu_item */
    GtkMenu *invite_submenu = GTK_MENU(gtk_menu_new());
    gtk_menu_item_set_submenu(invite_menu_item, GTK_WIDGET(invite_submenu));

    while (chats){
        item  = GTK_MENU_ITEM(gtk_menu_item_new_with_label(
                    srain_chat_get_chat_name(chats->data)));
        gtk_widget_show(GTK_WIDGET(item));
        gtk_widget_set_name(GTK_WIDGET(item), "invite_submenu_item");
        g_signal_connect(item, "activate",
                G_CALLBACK(nick_menu_item_on_activate), (char *)nick);
        gtk_menu_shell_append(GTK_MENU_SHELL(invite_submenu), GTK_WIDGET(item));

        chats = g_list_next(chats);
    }
    g_list_free(chats);

    gtk_menu_popup(nick_menu, NULL, NULL, NULL, NULL,
            event->button, event->time);

    g_object_unref(builder);
}
