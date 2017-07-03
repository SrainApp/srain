/**
 * @file srain_window.c
 * @brief Srain's main windows
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */


#include <gtk/gtk.h>
#include <string.h>
#include <assert.h>

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "theme.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chat.h"
#include "srain_stack_sidebar.h"
#include "tray_icon.h"

#include "meta.h"
#include "log.h"
#include "i18n.h"

typedef struct {
    // join_popover
    GtkEntry *join_chat_entry;
    GtkEntry *join_pwd_entry;
} JoinEntries;

typedef struct {
    // conn_popover
    GtkEntry *conn_addr_entry;
    GtkEntry *conn_port_entry;
    GtkEntry *conn_pwd_entry;
    GtkEntry *conn_nick_entry;
    GtkEntry *conn_real_entry;
    GtkCheckButton *conn_ssl_check_button;
    GtkCheckButton *conn_no_verfiy_check_button;
} ConnEntries;

struct _SrainWindow {
    GtkApplicationWindow parent;

    // Header
    GtkButton *about_button;
    GtkButton *conn_popover_button;
    GtkButton *join_popover_button;
    GtkSpinner *spinner;

    GtkBox *sidebar_box;
    SrainStackSidebar *sidebar;
    GtkStack *stack;
    GtkStatusIcon *tray_icon;
    GtkMenu *tray_menu;
    GtkMenuItem *about_menu_item;
    GtkMenuItem *quit_menu_item;

    // join_popover
    GtkPopover *join_popover;
    GtkEntry *join_chat_entry;
    GtkEntry *join_pwd_entry;
    GtkButton *join_button;
    JoinEntries join_entries;

    // conn_popover
    GtkPopover *conn_popover;
    GtkEntry *conn_addr_entry;
    GtkEntry *conn_port_entry;
    GtkEntry *conn_pwd_entry;
    GtkEntry *conn_nick_entry;
    GtkEntry *conn_real_entry;
    GtkCheckButton *conn_ssl_check_button;
    GtkCheckButton *conn_no_verfiy_check_button;
    GtkButton *conn_button;
    ConnEntries conn_entries;
};

struct _SrainWindowClass {
    GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(SrainWindow, srain_window, GTK_TYPE_APPLICATION_WINDOW);

static void quit_menu_item_on_activate(){
    srain_app_quit(srain_app);
}

static void monitor_active_window(GObject *object, GParamSpec *pspec,
        gpointer data ) {
   if (srain_window_is_active(SRAIN_WINDOW(object))){
       /* Stop stress the icon */
       srain_window_tray_icon_stress(SRAIN_WINDOW(object), 0);
   } else {

   }
}

static void show_about_dialog(gpointer user_data){
    GtkWidget *window = user_data;
    const gchar *authors[] = { PACKAGE_AUTHOR " <" PACKAGE_EMAIL ">", NULL };
    const gchar **documentors = authors;
    const gchar *version = g_strdup_printf(_("%s%s\nRunning against GTK+ %d.%d.%d"),
            PACKAGE_VERSION,
            PACKAGE_BUILD,
            gtk_get_major_version(),
            gtk_get_minor_version(),
            gtk_get_micro_version());

    gtk_show_about_dialog(GTK_WINDOW(window),
            "program-name", PACKAGE_NAME,
            "version", version,
            "copyright", "(C) " PACKAGE_COPYRIGHT_DATES " " PACKAGE_AUTHOR,
            "license-type", GTK_LICENSE_GPL_3_0,
            "website", PACKAGE_WEBSITE,
            "comments", PACKAGE_DESC,
            "authors", authors,
            "documenters", documentors,
            "logo-icon-name", "srain",
            "title", _("About Srain"),
            NULL);
}

static void popover_button_on_click(gpointer user_data){
    GtkPopover *popover;

    popover = user_data;
    gtk_widget_set_visible(GTK_WIDGET(popover),
            !gtk_widget_get_visible(GTK_WIDGET(popover)));
}

static void popover_entry_on_activate(GtkWidget *widget, gpointer user_data){
    GtkWidget *parent;
    GtkEntry *entry;
    GtkButton *button;

    entry = GTK_ENTRY(widget);
    button = user_data;

    parent = gtk_widget_get_parent(GTK_WIDGET(entry));

    // Move focus to next entry, if reach the last one, "click" the button
    if (!gtk_widget_child_focus(GTK_WIDGET(parent), GTK_DIR_TAB_FORWARD))
        g_signal_emit_by_name(button, "clicked");
}

static void join_button_on_click(gpointer user_data){
    int count;
    const char *name;
    const char *pwd;
    JoinEntries *join_entries;
    SrainChat *chat;
    const char *params[2];

    join_entries = user_data;

    name = gtk_entry_get_text(join_entries->join_chat_entry);
    pwd = gtk_entry_get_text(join_entries->join_pwd_entry);
    chat = srain_window_get_cur_chat(srain_win);

    g_return_if_fail(chat);

    count = 0;
    params[count++] = name;
    params[count++] = pwd;

    sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_JOIN, params, count);

    gtk_entry_set_text(join_entries->join_chat_entry, "");
    gtk_entry_set_text(join_entries->join_pwd_entry, "");
}

static void conn_button_on_click(gpointer user_data){
    int count;
    bool ssl;
    bool notverify;
    const char *addr;
    const char *port;
    const char *passwd;
    const char *nick;
    const char *realname;
    ConnEntries *conn_entries;
    const char *params[9];

    conn_entries = user_data;

    addr = gtk_entry_get_text(conn_entries->conn_addr_entry);
    port = gtk_entry_get_text(conn_entries->conn_port_entry);
    passwd = gtk_entry_get_text(conn_entries->conn_pwd_entry);
    nick = gtk_entry_get_text(conn_entries->conn_nick_entry);
    realname = gtk_entry_get_text(conn_entries->conn_real_entry);
    ssl = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(conn_entries->conn_ssl_check_button));
    notverify = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(conn_entries->conn_no_verfiy_check_button));

    if (strlen(addr) == 0) return;
    if (strlen(nick) == 0) return;

    count = 0;
    params[count++] = addr;
    params[count++] = addr;
    params[count++] = port;
    params[count++] = passwd;
    if (ssl) {
        params[count++] = "TRUE";
    } else {
        params[count++] = "FALSE";
    }
    if (notverify) {
        params[count++] = "TRUE";
    } else {

        params[count++] = "FALSE";
    }
    params[count++] = "UTF-8";
    params[count++] = nick;
    params[count++] = realname;

    sui_event_hdr(NULL, SUI_EVENT_CONNECT, params, count);

    gtk_entry_set_text(conn_entries->conn_addr_entry, "");
    gtk_entry_set_text(conn_entries->conn_port_entry, "");
    gtk_entry_set_text(conn_entries->conn_pwd_entry, "");
    gtk_entry_set_text(conn_entries->conn_nick_entry, "");
    gtk_entry_set_text(conn_entries->conn_real_entry, "");
}

static gboolean CTRL_J_K_on_press(GtkAccelGroup *group, GObject *obj,
        guint keyval, GdkModifierType mod, gpointer user_data){
    SrainStackSidebar *sidebar;

    if (mod != GDK_CONTROL_MASK) return FALSE;

    sidebar = user_data;
    switch (keyval){
        case GDK_KEY_k:
            srain_stack_sidebar_prev(sidebar);
            break;
        case GDK_KEY_j:
            srain_stack_sidebar_next(sidebar);
            break;
        default:
            ERR_FR("unknown keyval %d", keyval);
            return FALSE;
    }

    return TRUE;
}

static void srain_window_init(SrainWindow *self){
    GClosure *closure_j;
    GClosure *closure_k;
    GtkAccelGroup *accel;

    gtk_widget_init_template(GTK_WIDGET(self));

    // Filling entries
    self->join_entries.join_chat_entry = self->join_chat_entry;
    self->join_entries.join_pwd_entry = self->join_pwd_entry;
    self->conn_entries.conn_addr_entry = self->conn_addr_entry;
    self->conn_entries.conn_port_entry = self->conn_port_entry;
    self->conn_entries.conn_pwd_entry = self->conn_pwd_entry;
    self->conn_entries.conn_nick_entry = self->conn_nick_entry;
    self->conn_entries.conn_real_entry = self->conn_real_entry;
    self->conn_entries.conn_ssl_check_button = self->conn_ssl_check_button;
    self->conn_entries.conn_no_verfiy_check_button = self->conn_no_verfiy_check_button;

    /* stack sidebar init */
    self->sidebar = srain_stack_sidebar_new();
    gtk_widget_show(GTK_WIDGET(self->sidebar));
    gtk_box_pack_start(self->sidebar_box, GTK_WIDGET(self->sidebar),
            TRUE, TRUE, 0);
    srain_stack_sidebar_set_stack(self->sidebar, self->stack);

    theme_apply(GTK_WIDGET(self));
    theme_apply(GTK_WIDGET(self->tray_menu));

    tray_icon_set_callback(self->tray_icon, self, self->tray_menu);
    g_signal_connect_swapped(self->quit_menu_item, "activate",
            G_CALLBACK(quit_menu_item_on_activate), NULL);
    g_signal_connect_swapped(self->about_menu_item, "activate",
            G_CALLBACK(show_about_dialog), self);


    /* :-| OK 为什么我没看到 is-active 属性 和 notify 的用法 */
    g_signal_connect(self, "notify::is-active",
            G_CALLBACK(monitor_active_window), NULL);

    // Click to show/hide GtkPopover
    g_signal_connect_swapped(self->about_button, "clicked",
            G_CALLBACK(show_about_dialog), self);
    g_signal_connect_swapped(self->join_popover_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->join_popover);
    g_signal_connect_swapped(self->conn_popover_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->conn_popover);
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->join_popover);
    g_signal_connect_swapped(self->conn_button, "clicked",
            G_CALLBACK(popover_button_on_click), self->conn_popover);

    // Click to submit entries' messages
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(join_button_on_click), &(self->join_entries));
    g_signal_connect_swapped(self->conn_button, "clicked",
            G_CALLBACK(conn_button_on_click), &(self->conn_entries));

    // Press ENTER to move focus or submit entries' messages
    g_signal_connect(self->join_chat_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->join_button);
    g_signal_connect(self->join_pwd_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->join_button);
    g_signal_connect(self->conn_addr_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->conn_button);
    g_signal_connect(self->conn_port_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->conn_button);
    g_signal_connect(self->conn_pwd_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->conn_button);
    g_signal_connect(self->conn_nick_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->conn_button);
    g_signal_connect(self->conn_real_entry, "activate",
            G_CALLBACK(popover_entry_on_activate), self->conn_button);

    /* shortcut <C-j> and <C-k> */
    accel = gtk_accel_group_new();

    closure_j = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press),
            self->sidebar, NULL);
    closure_k = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press),
            self->sidebar, NULL);

    gtk_accel_group_connect(accel, GDK_KEY_j, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_j);
    gtk_accel_group_connect(accel, GDK_KEY_k, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_k);

    gtk_window_add_accel_group(GTK_WINDOW(self), accel);

    g_closure_unref(closure_j);
    g_closure_unref(closure_k);
}

static void srain_window_class_init(SrainWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, about_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_popover_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_popover_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, spinner);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, stack);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_icon);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, quit_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, about_menu_item);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, sidebar_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_popover);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_popover);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_chat_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_pwd_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_addr_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_port_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_pwd_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_nick_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_real_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_ssl_check_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_no_verfiy_check_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, conn_button);
}

SrainWindow* srain_window_new(SrainApp *app){
    return g_object_new(SRAIN_TYPE_WINDOW, "application", app, NULL);
}

SrainChat* srain_window_add_chat(SrainWindow *win, SuiSession *sui,
        const char *name, const char *remark, ChatType type){
    SrainChat *chat;

    if (srain_window_get_chat(win, name, remark)){
        ERR_FR("SrainChat name: %s, remark: %s already exist",
                name, remark);
        return NULL;
    }

    chat = srain_chat_new(sui, name, remark, type);

    GString *gstr = g_string_new("");
    g_string_printf(gstr, "%s %s", remark, name);
    gtk_stack_add_named(win->stack, GTK_WIDGET(chat), gstr->str);
    g_string_free(gstr, TRUE);

    theme_apply(GTK_WIDGET(win));

    gtk_stack_set_visible_child(win->stack, GTK_WIDGET(chat));
    return chat;
}

void srain_window_rm_chat(SrainWindow *win, SrainChat *chat){
    srain_user_list_clear(srain_chat_get_user_list(chat));
    gtk_container_remove(GTK_CONTAINER(win->stack), GTK_WIDGET(chat));
}

SrainChat* srain_window_get_cur_chat(SrainWindow *win){
    SrainChat *chat = NULL;

    chat = SRAIN_CHAT(gtk_stack_get_visible_child(win->stack));

    // TODO:
    //  if (chat == NULL) ERR_FR("no visible chat");

    return chat;
}

SrainChat* srain_window_get_chat(SrainWindow *win,
        const char *name, const char *remark){
    SrainChat *chat = NULL;

    GString *fullname = g_string_new("");
    g_string_printf(fullname, "%s %s", remark, name);
    chat = SRAIN_CHAT(gtk_stack_get_child_by_name(win->stack, fullname->str));
    g_string_free(fullname, TRUE);

    return chat;
}

/**
 * @brief Find out all SrainChats with the server_name given as argument
 *
 * @param win
 * @param server_name
 *
 * @return a GList, may be NULL, should be freed by caller
 */
GList* srain_window_get_chats_by_remark(SrainWindow *win, const char *remark){
    GList *all_chats;
    GList *chats = NULL;
    SrainChat *chat = NULL;

    all_chats = gtk_container_get_children(GTK_CONTAINER(win->stack));
    while (all_chats){
        chat = SRAIN_CHAT(all_chats->data);

        if (strcmp(remark, srain_chat_get_remark(chat)) == 0){
            chats = g_list_append(chats, chat);
        }
        all_chats = g_list_next(all_chats);
    }

    return chats;
}

void srain_window_spinner_toggle(SrainWindow *win, gboolean is_busy){
   is_busy
        ? gtk_spinner_start(win->spinner)
        : gtk_spinner_stop(win->spinner);
}

void srain_window_stack_sidebar_update(SrainWindow *win, SrainChat *chat,
        const char *nick, const char *msg){
    if (SRAIN_CHAT(gtk_stack_get_visible_child(win->stack)) != chat){
        srain_stack_sidebar_update(win->sidebar, chat, nick, msg, 0);
    } else {
        srain_stack_sidebar_update(win->sidebar, chat, nick, msg, 1);
    }
}

void srain_window_tray_icon_stress(SrainWindow *win, int stress){
    gtk_status_icon_set_from_icon_name(win->tray_icon, stress ? "srain-red": "srain");
}

int srain_window_is_active(SrainWindow *win){
    int active;

    g_object_get(G_OBJECT(win), "is-active", &active, NULL);

    return active;
}
