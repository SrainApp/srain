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

/**
 * @file sui_join_panel.c
 * @brief Panel widget for join and search channel
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-09-16
 */

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "config/password.h"
#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "utils.h"

#include "sui_common.h"
#include "sui_event_hdr.h"
#include "sui_window.h"
#include "sui_buffer.h"
#include "sui_chat_buffer.h"
#include "sui_server_buffer.h"
#include "gtk_compat.h"
#include "sui_join_panel.h"

#define PAGE_JOIN_CHANNEL           "join_channel_page"
#define PAGE_SEARCH_CHANNEL         "search_channel_page"

#define MATCH_CHANNEL               0
#define MATCH_CHANNEL_WITH_REGEX    1
#define MATCH_TOPIC_WITH_REGEX      2

#define MATCH_LIST_STORE_COL_INDEX      0
#define MATCH_LIST_STORE_COL_COMMENT    1

#define CHANNEL_LIST_STORE_COL_CHANNEL  0
#define CHANNEL_LIST_STORE_COL_USERS    1
#define CHANNEL_LIST_STORE_COL_TOPIC    2

struct _SuiJoinPanel {
    GtkBox parent;

    bool is_adding; // Whether adding channels
    int match;

    GtkStack *stack;

    /* Join channel page */
    GtkEntry *chan_entry;
    GtkEntry *password_entry;
    GtkCheckButton *remember_password_check_button;

    /* Search channel page */
    GtkEntry *search_entry;
    GtkButton *refresh_button;
    /* Filter */
#if GTK_MAJOR_VERSION >= 4
    GtkDropDown *match_combo_box;
#else
    GtkComboBox *match_combo_box;
#endif
    GtkSpinButton *min_users_spin_button;
    GtkSpinButton *max_users_spin_button;
    /* Channel list */
#if GTK_MAJOR_VERSION >= 4
    GtkListBox *chan_list_box;
#else
    GtkTreeView *chan_tree_view;
    GtkTreeViewColumn *chan_tree_view_column;
    GtkTreeViewColumn *users_tree_view_column;
    GtkTreeViewColumn *topic_tree_view_column;
#endif
    /* Channel list models */
#if GTK_MAJOR_VERSION >= 4
    GListModel *chan_list_model;
#else
    GtkTreeModel *chan_tree_model;
    GtkTreeModelFilter *chan_tree_model_filter;
    GtkTreeModelSort *chan_tree_model_sorter;
#endif
    /* Status */
    GtkLabel *status_label;
    GtkSpinner *status_spinner;

    /* Buttons */
    GtkButton *cancel_button;
    GtkButton *join_button;

    /* Data model */
#if GTK_MAJOR_VERSION >= 4
    GtkStringList *match_list_store;
#else
    GtkListStore *match_list_store;
#endif
};

struct _SuiJoinPanelClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SuiJoinPanel, sui_join_panel, GTK_TYPE_BOX);

static void match_combo_box_set_model(SuiJoinPanel *self);

static void cancel_button_on_click(gpointer user_data);
static void join_button_on_click(gpointer user_data);
#if GTK_MAJOR_VERSION >= 4
static void match_combo_box_on_changed(GObject *object, GParamSpec *pspec,
        gpointer user_data);
static void match_combo_box_refilter_on_changed(GObject *object,
        GParamSpec *pspec, gpointer user_data);
#else
static void match_combo_box_on_changed(GtkComboBox *combobox,
        gpointer user_data);
#endif
static void refresh_button_on_clicked(gpointer user_data);
#if GTK_MAJOR_VERSION >= 4
static void chan_list_box_on_row_activated(GtkListBox *box, GtkListBoxRow *row,
        gpointer user_data);
static void rebuild_chan_list_box(SuiJoinPanel *self);
static const char *get_selected_channel(SuiJoinPanel *self);
static GtkWidget *new_channel_row(const char *chan, int users, const char *topic);
static gboolean chan_item_visible(SuiJoinPanel *self, const char *chan,
        int users, const char *topic);
static void chan_list_model_on_items_changed(GListModel *model,
        guint position, guint removed, guint added, gpointer user_data);
#else
static void chan_tree_view_on_row_activate(GtkTreeView *view,
        GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
#endif
gboolean chan_tree_visible_func(GtkTreeModel *model, GtkTreeIter *iter,
        gpointer user_data);
static void chan_tree_model_filter_refilter(gpointer user_data);
#if GTK_MAJOR_VERSION < 4
static void chan_tree_model_on_row_changed(GtkTreeModel *tree_model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
#endif
static void chan_entry_on_changed(GtkEditable *editable, gpointer user_data);
static void on_password_lookup(GObject *source, GAsyncResult *result,
        gpointer user_data);
static void stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data);

static void update_status(SuiJoinPanel *self);
static void update_focus(SuiJoinPanel *self);

static const int match_modes[] = {
    MATCH_CHANNEL,
};

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

static void sui_join_panel_init(SuiJoinPanel *self){
    gtk_widget_init_template(GTK_WIDGET(self));

#if GTK_CHECK_VERSION(3, 18, 0)
    gtk_stack_set_interpolate_size(self->stack, TRUE);
#endif

    self->match = MATCH_CHANNEL;
    match_combo_box_set_model(self);

    g_signal_connect(self->stack, "notify::visible-child",
            G_CALLBACK(stack_on_child_changed), self);

    /* Press enter to connect */
    g_signal_connect_swapped(self->chan_entry, "activate",
            G_CALLBACK(join_button_on_click), self);
    g_signal_connect_swapped(self->password_entry, "activate",
            G_CALLBACK(join_button_on_click), self);
    g_signal_connect_swapped(self->search_entry, "activate",
            G_CALLBACK(join_button_on_click), self);

    g_signal_connect_swapped(self->refresh_button, "clicked",
            G_CALLBACK(refresh_button_on_clicked), self);
#if GTK_MAJOR_VERSION >= 4
    g_signal_connect(self->match_combo_box, "notify::selected",
            G_CALLBACK(match_combo_box_on_changed), self);
#else
    g_signal_connect(self->match_combo_box, "changed",
            G_CALLBACK(match_combo_box_on_changed), self);
#endif
#if GTK_MAJOR_VERSION >= 4
    g_signal_connect(self->chan_list_box, "row-activated",
            G_CALLBACK(chan_list_box_on_row_activated), self);
#else
    g_signal_connect(self->chan_tree_view, "row-activated",
            G_CALLBACK(chan_tree_view_on_row_activate), self);
#endif
    g_signal_connect(self->chan_entry, "changed",
            G_CALLBACK(chan_entry_on_changed), self);

    /* Filter condition changed */
    g_signal_connect_swapped(self->search_entry, "changed",
            G_CALLBACK(chan_tree_model_filter_refilter), self);
#if GTK_MAJOR_VERSION >= 4
    g_signal_connect(self->match_combo_box, "notify::selected",
            G_CALLBACK(match_combo_box_refilter_on_changed), self);
#else
    g_signal_connect_swapped(self->match_combo_box, "changed",
            G_CALLBACK(chan_tree_model_filter_refilter), self);
#endif
    g_signal_connect_swapped(self->min_users_spin_button, "changed",
            G_CALLBACK(chan_tree_model_filter_refilter), self);
    g_signal_connect_swapped(self->max_users_spin_button, "changed",
            G_CALLBACK(chan_tree_model_filter_refilter), self);
    g_signal_connect_swapped(self->max_users_spin_button, "changed",
            G_CALLBACK(chan_tree_model_filter_refilter), self);

    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(join_button_on_click), self);
}

static void sui_join_panel_class_init(SuiJoinPanelClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/join_panel.ui");
#else
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/join_panel.glade");
#endif

    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, stack);

    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, chan_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, password_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, remember_password_check_button);

    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, search_entry);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, refresh_button);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, match_combo_box);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, min_users_spin_button);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, max_users_spin_button);

#if GTK_MAJOR_VERSION >= 4
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, chan_list_box);
#else
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, chan_tree_view);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, chan_tree_view_column);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, users_tree_view_column);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, topic_tree_view_column);
#endif

    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, status_label);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, status_spinner);

    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, cancel_button);
    gtk_widget_class_bind_template_child(widget_class, SuiJoinPanel, join_button);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiJoinPanel* sui_join_panel_new(){
    return g_object_new(SUI_TYPE_JOIN_PANEL, NULL);
}

void sui_join_panel_clear(SuiJoinPanel *self){
    /* Clear join channel page input */
    srn_gtk_entry_set_text(self->chan_entry, "");
    srn_gtk_entry_set_text(self->password_entry, "");
    srn_gtk_check_button_set_active(
            self->remember_password_check_button, FALSE);

    /* Clear search channel page input */
    srn_gtk_entry_set_text(self->search_entry, "");
    gtk_spin_button_set_value(self->min_users_spin_button, -1);
    gtk_spin_button_set_value(self->max_users_spin_button, -1);
}

#if GTK_MAJOR_VERSION >= 4
void sui_join_panel_set_list_model(SuiJoinPanel *self, GListModel *model){
    g_return_if_fail(!self->chan_list_model);

    self->chan_list_model = g_object_ref(model);
    g_signal_connect(self->chan_list_model, "items-changed",
            G_CALLBACK(chan_list_model_on_items_changed), self);
    rebuild_chan_list_box(self);
    update_status(self);
}
#else
void sui_join_panel_set_model(SuiJoinPanel *self, GtkTreeModel *model){
    g_return_if_fail(!self->chan_tree_model
            && !self->chan_tree_model_filter
            && !self->chan_tree_model_sorter);

    /* Init model */
    self->chan_tree_model = model;
    g_signal_connect(self->chan_tree_model, "row-inserted",
            G_CALLBACK(chan_tree_model_on_row_changed), self);
    g_signal_connect(self->chan_tree_model, "row-changed",
            G_CALLBACK(chan_tree_model_on_row_changed), self);

    /* Init filter */
    self->chan_tree_model_filter =
        GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(model, NULL));
    gtk_tree_model_filter_set_visible_func(self->chan_tree_model_filter,
            chan_tree_visible_func, self, NULL);

    /* Init sorter */
    self->chan_tree_model_sorter =
        GTK_TREE_MODEL_SORT(gtk_tree_model_sort_new_with_model(
                    GTK_TREE_MODEL(self->chan_tree_model_filter)));
    gtk_tree_view_set_model(self->chan_tree_view,
            GTK_TREE_MODEL(self->chan_tree_model_sorter));
}
#endif

void sui_join_panel_set_is_adding(SuiJoinPanel *self, bool is_adding) {
    self->is_adding = is_adding;
    update_status(self);
}

bool sui_join_panel_get_is_adding(SuiJoinPanel *self){
    return self->is_adding;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void match_combo_box_set_model(SuiJoinPanel *self){
#if GTK_MAJOR_VERSION >= 4
    GtkStringList *store;
    GtkDropDown *combobox;
    GtkExpression *expression;
#else
    GtkListStore *store;
    GtkComboBox *combobox;
    GtkTreeIter iter;
#endif

    /* 2 columns: index, comment */
#if GTK_MAJOR_VERSION >= 4
    self->match_list_store = gtk_string_list_new(NULL);
#else
    self->match_list_store = gtk_list_store_new(2,
            G_TYPE_INT,
            G_TYPE_STRING);
#endif
    store = self->match_list_store;
    combobox = self->match_combo_box;

#if GTK_MAJOR_VERSION >= 4
    gtk_string_list_append(store, _("Match channel name"));

    expression = gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, NULL,
            "string");
    gtk_drop_down_set_model(combobox, G_LIST_MODEL(store));
    gtk_drop_down_set_expression(combobox, expression);
    gtk_drop_down_set_selected(combobox, 0);
    gtk_expression_unref(expression);
#else
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            MATCH_LIST_STORE_COL_INDEX, MATCH_CHANNEL,
            MATCH_LIST_STORE_COL_COMMENT, _("Match channel name"),
            -1);

    /* TODO
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            MATCH_LIST_STORE_COL_INDEX, MATCH_CHANNEL_WITH_REGEX,
            MATCH_LIST_STORE_COL_COMMENT, _("Match channel name with regular expression"),
            -1);
    */

    gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
#endif
}

static void join_button_on_click(gpointer user_data){
    const char *page;
    const char *chan;
    const char *passwd;
    bool rmb_passwd;
    GVariantDict *params;
    g_autofree char *_chan = NULL;
    SrnRet ret;
    SuiJoinPanel *self;
    SuiBuffer *buf;

    self = user_data;
    buf = sui_common_get_cur_buffer(); // FIXME
    if (!SUI_IS_BUFFER(buf)){
       sui_message_box(_("Error"),
               _("Please connect to a server before joining any channel"));
       return;
    }

    page = gtk_stack_get_visible_child_name(self->stack);
    if (g_strcmp0(page, PAGE_JOIN_CHANNEL) == 0){
        chan = srn_gtk_entry_get_text(self->chan_entry);
        passwd = srn_gtk_entry_get_text(self->password_entry);
        rmb_passwd = srn_gtk_check_button_get_active(
                self->remember_password_check_button);
    } else if (g_strcmp0(page, PAGE_SEARCH_CHANNEL) == 0){
        const char *selected_chan;

        selected_chan = get_selected_channel(self);
        if (selected_chan){
            chan = selected_chan;
        } else {
            chan = srn_gtk_entry_get_text(self->search_entry);
        }
        passwd = "";
        rmb_passwd = FALSE;
    } else {
        g_return_if_reached();
    }

    if (rmb_passwd) {
        const char *srv_name;
        SrnApplication *app_model;
        SrnConfigManager *cfg_mgr;
        SrnChat *chat;

        app_model = sui_application_get_ctx(sui_application_get_instance());
        cfg_mgr = app_model->cfg_mgr;

        // TODO: Better way to get server name?
        chat = sui_buffer_get_ctx(sui_common_get_cur_buffer());
        srv_name = chat->srv->name;

        if (strlen(passwd)) { // Reqeust to store password
            SrnRet ret;

            ret = srn_config_manager_store_channel_password(cfg_mgr, passwd,
                    srv_name, chan);
            if (!RET_IS_OK(ret)) {
                ret = RET_ERR(_("Failed to store channel password: %1$s"),
                        RET_MSG(ret));
                sui_message_box(_("Error"), RET_MSG(ret) );
                // No need to return
            }
        } else { // Reqeust to remove password
            SrnRet ret;

            ret = srn_config_manager_clear_channel_password(cfg_mgr,
                    chat->srv->name, chan);
            if (!RET_IS_OK(ret)) {
                ret = RET_ERR(_("Failed to clear channel password: %1$s"),
                        RET_MSG(ret));
                sui_message_box(_("Error"), RET_MSG(ret) );
                // No need to return
            }
        }
    }

    // TODO: Deprecate SUI_EVENT_JOIN, use srn_server_add_chat_with_config
    params = g_variant_dict_new(NULL);
    g_variant_dict_insert(params, "channel", SUI_EVENT_PARAM_STRING, chan);
    g_variant_dict_insert(params, "password", SUI_EVENT_PARAM_STRING, passwd);

    ret = sui_buffer_event_hdr(buf, SUI_EVENT_JOIN, params);
    g_variant_dict_unref(params);

    if (RET_IS_OK(ret)){
        srn_gtk_button_clicked(self->cancel_button);
    } else {
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void cancel_button_on_click(gpointer user_data){
    SuiJoinPanel *self;

    self = user_data;

    sui_common_popdown_panel(GTK_WIDGET(self));
    sui_join_panel_clear(self);
}

#if GTK_MAJOR_VERSION >= 4
static void match_combo_box_on_changed(GObject *object, GParamSpec *pspec,
        gpointer user_data){
    guint position;
    SuiJoinPanel *self;

    (void)object;
    (void)pspec;

    self = user_data;
    position = gtk_drop_down_get_selected(self->match_combo_box);
    if (position == GTK_INVALID_LIST_POSITION ||
            position >= G_N_ELEMENTS(match_modes)){
        ERR_FR("No active item");
        return;
    }

    self->match = match_modes[position];

    DBG_FR("Selected index: %d", self->match);
}

static void match_combo_box_refilter_on_changed(GObject *object,
        GParamSpec *pspec, gpointer user_data){
    (void)object;
    (void)pspec;

    chan_tree_model_filter_refilter(user_data);
}
#else
static void match_combo_box_on_changed(GtkComboBox *combobox,
        gpointer user_data){
    int match;
    GtkTreeIter iter;
    SuiJoinPanel *self;

    self = user_data;

    if (!gtk_combo_box_get_active_iter(combobox, &iter)){
        ERR_FR("No acive item");
    }

    gtk_tree_model_get(GTK_TREE_MODEL(self->match_list_store), &iter,
            MATCH_LIST_STORE_COL_INDEX, &match,
            -1);
    self->match = match;

    DBG_FR("Selected index: %d", match);
}
#endif

static void refresh_button_on_clicked(gpointer user_data){
    SrnRet ret;
    SuiBuffer *buf;

    buf = sui_common_get_cur_buffer(); // FIXME:
    if (!SUI_IS_BUFFER(buf)){
       sui_message_box(_("Error"), _("Please connect to a server before searching any channel"));
       return;
    }

    ret = sui_buffer_event_hdr(buf, SUI_EVENT_CHAN_LIST, NULL);
    if (!RET_IS_OK(ret)){
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

#if GTK_MAJOR_VERSION >= 4
static void chan_list_box_on_row_activated(GtkListBox *box, GtkListBoxRow *row,
        gpointer user_data){
    const char *chan;
    SuiJoinPanel *self;

    self = user_data;
    chan = g_object_get_data(G_OBJECT(row), "channel");
    g_return_if_fail(chan);

    srn_gtk_entry_set_text(self->chan_entry, chan);
    gtk_stack_set_visible_child_name(self->stack, PAGE_JOIN_CHANNEL);
}
#else
static void chan_tree_view_on_row_activate(GtkTreeView *view,
        GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data){
    char *chan = NULL;
    GtkTreeIter iter;
    GtkTreeModel *model;
    SuiJoinPanel *self;

    self = user_data;
    model = gtk_tree_view_get_model(view);
    if (!gtk_tree_model_get_iter(model, &iter, path)){
        ERR_FR("Failed to get GtkTreeIter from GtkTreePath");
        return;
    }

    gtk_tree_model_get(model, &iter,
            CHANNEL_LIST_STORE_COL_CHANNEL, &chan,
            -1);
    g_return_if_fail(chan);

    /* If a row is activated, set the value of chan_entry and switch to
     * PAGE_JOIN_CHANNEL. */
    srn_gtk_entry_set_text(self->chan_entry, chan);
    gtk_stack_set_visible_child_name(self->stack, PAGE_JOIN_CHANNEL);

    g_free(chan);
}
#endif

static void chan_tree_model_filter_refilter(gpointer user_data){
    char *status;
    SuiJoinPanel *self;
    int cur;
    int max;

    self = user_data;

#if GTK_MAJOR_VERSION < 4
    GtkTreeModel *model;
    GtkTreeModelFilter *filter;

    model = self->chan_tree_model;
    filter = self->chan_tree_model_filter;
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(filter));
    /* Update status while all channels have loaded */
    cur = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(filter), NULL);
    max = model ? gtk_tree_model_iter_n_children(model, NULL) : 0;
#else
    rebuild_chan_list_box(self);
    cur = 0;
    for (GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(self->chan_list_box));
            child != NULL;
            child = gtk_widget_get_next_sibling(child)){
        cur++;
    }
    max = self->chan_list_model ? g_list_model_get_n_items(self->chan_list_model) : 0;
#endif
    status = g_strdup_printf(_("Showing %1$d of %2$d channels"), cur, max);
    gtk_label_set_text(self->status_label, status);
    g_free(status);
}

#if GTK_MAJOR_VERSION < 4
gboolean chan_tree_visible_func(GtkTreeModel *model, GtkTreeIter *iter,
        gpointer user_data){
    int users;
    int min_users;
    int max_users;
    char *chan = NULL;
    char *topic = NULL;
    const char *input;
    gboolean visable;
    SuiJoinPanel *self;

    visable = FALSE;
    self = user_data;
    gtk_tree_model_get(model, iter,
            CHANNEL_LIST_STORE_COL_CHANNEL, &chan,
            CHANNEL_LIST_STORE_COL_USERS, &users,
            CHANNEL_LIST_STORE_COL_TOPIC, &topic,
            -1);

    DBG_FR("cha: %s, users: %d, topic: %s", chan, users, topic);

    if (!chan) {
        goto FIN;
    }

    min_users = gtk_spin_button_get_value(self->min_users_spin_button);
    max_users = gtk_spin_button_get_value(self->max_users_spin_button);
    input = srn_gtk_entry_get_text(self->search_entry);

    /* Filter users */
    if (min_users != - 1 && users < min_users){
        goto FIN;
    }
    if (max_users != - 1 && users > max_users){
        goto FIN;
    }

    switch (self->match){
        case MATCH_CHANNEL:
            if (str_is_empty(input) || g_strstr_len(chan, -1, input) != NULL){
                visable = TRUE;
            }
            break;
        case MATCH_CHANNEL_WITH_REGEX:
        default:
            ERR_FR("Unsupported match type");
    }

FIN:
    if (chan){
        g_free(chan);
    }
    if (topic){
        g_free(topic);
    }

    DBG_FR("visable: %d", visable);

    return visable;
}
#endif

#if GTK_MAJOR_VERSION < 4
static void chan_tree_model_on_row_changed(GtkTreeModel *tree_model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer user_data){
    SuiJoinPanel *self;

    self = SUI_JOIN_PANEL(user_data);

#if GTK_MAJOR_VERSION >= 4
    rebuild_chan_list_box(self);
#endif
    update_status(self);
}
#endif

static void update_status(SuiJoinPanel *self){
    int cur;
    int max;
    char *status;

    if (self->is_adding) {
        gtk_spinner_start(self->status_spinner);
    } else {
        gtk_spinner_stop(self->status_spinner);
    }

    if (
#if GTK_MAJOR_VERSION >= 4
            self->chan_list_model
#else
            self->chan_tree_model
#endif
       ) {
#if GTK_MAJOR_VERSION >= 4
        cur = 0;
        for (GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(self->chan_list_box));
                child != NULL;
                child = gtk_widget_get_next_sibling(child)){
            cur++;
        }
        max = g_list_model_get_n_items(self->chan_list_model);
#else
        cur = gtk_tree_model_iter_n_children(
                GTK_TREE_MODEL(self->chan_tree_model_filter), NULL);
        max = gtk_tree_model_iter_n_children(self->chan_tree_model, NULL);
#endif
    } else {
        cur = 0;
        max = 0;
    }

    status  = g_strdup_printf(_("Showing %1$d of %2$d channels"), cur, max);
    gtk_label_set_text(self->status_label, status);
    g_free(status);
}

static void chan_entry_on_changed(GtkEditable *editable, gpointer user_data) {
    const char *srv_name;
    const char *chan_name;
    GtkEntry *entry;
    SuiJoinPanel *self;
    SrnApplication *app_model;
    SrnConfigManager *cfg_mgr;
    SrnChat *chat;

    entry = GTK_ENTRY(editable);
    self = SUI_JOIN_PANEL(user_data);
    app_model = sui_application_get_ctx(sui_application_get_instance());
    cfg_mgr = app_model->cfg_mgr;

    // TODO: Better way to get server name?
    chat = sui_buffer_get_ctx(sui_common_get_cur_buffer());
    srv_name = chat->srv->name;

    chan_name = srn_gtk_entry_get_text(entry);

    // Clear channel password when channel name is not valid
    if (str_is_empty(chan_name)) {
        srn_gtk_entry_set_text(self->password_entry, "");
        return;
    }

    // Lookup channel password asynchronously
    secret_password_lookup(srn_config_manager_get_channel_secret_schema(cfg_mgr),
            NULL, on_password_lookup, self->password_entry,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_CHANNEL, chan_name,
            NULL);
}

static void on_password_lookup(GObject *source, GAsyncResult *result,
        gpointer user_data) {
    char *passwd;
    GtkEntry *entry;

    entry = GTK_ENTRY(user_data);

    passwd = secret_password_lookup_finish(result, NULL);
    if (!passwd) {
        return;
    }

    srn_gtk_entry_set_text(entry, passwd);
    secret_password_free(passwd);
}

static void stack_on_child_changed(GtkWidget *widget, GParamSpec *pspec,
        gpointer user_data) {
    update_focus(SUI_JOIN_PANEL(user_data));
}

static void update_focus(SuiJoinPanel *self) {
    const char *page = gtk_stack_get_visible_child_name(self->stack);

    if (g_strcmp0(page, PAGE_JOIN_CHANNEL) == 0){
        gtk_widget_grab_focus(GTK_WIDGET(self->chan_entry));
    } else if (g_strcmp0(page, PAGE_SEARCH_CHANNEL) == 0){
        gtk_widget_grab_focus(GTK_WIDGET(self->search_entry));
    } else {
        g_warn_if_reached();
    }
}

#if GTK_MAJOR_VERSION >= 4
static gboolean chan_item_visible(SuiJoinPanel *self, const char *chan,
        int users, const char *topic){
    int min_users;
    int max_users;
    const char *input;

    (void)topic;

    DBG_FR("cha: %s, users: %d, topic: %s", chan, users, topic);

    if (!chan) {
        return FALSE;
    }

    min_users = gtk_spin_button_get_value(self->min_users_spin_button);
    max_users = gtk_spin_button_get_value(self->max_users_spin_button);
    input = srn_gtk_entry_get_text(self->search_entry);

    if (min_users != -1 && users < min_users){
        return FALSE;
    }
    if (max_users != -1 && users > max_users){
        return FALSE;
    }

    switch (self->match){
        case MATCH_CHANNEL:
            return str_is_empty(input) || g_strstr_len(chan, -1, input) != NULL;
        case MATCH_CHANNEL_WITH_REGEX:
        default:
            ERR_FR("Unsupported match type");
            return FALSE;
    }
}

static void chan_list_model_on_items_changed(GListModel *model,
        guint position, guint removed, guint added, gpointer user_data){
    SuiJoinPanel *self;

    (void)model;
    (void)position;
    (void)removed;
    (void)added;

    self = SUI_JOIN_PANEL(user_data);
    rebuild_chan_list_box(self);
    update_status(self);
}

static void rebuild_chan_list_box(SuiJoinPanel *self){
    while (gtk_widget_get_first_child(GTK_WIDGET(self->chan_list_box))){
        gtk_list_box_remove(self->chan_list_box,
                gtk_widget_get_first_child(GTK_WIDGET(self->chan_list_box)));
    }

    if (!self->chan_list_model){
        return;
    }

    for (guint i = 0; i < g_list_model_get_n_items(self->chan_list_model); i++){
        SuiChannelListItem *item;
        const char *chan;
        const char *topic;
        int users;

        item = g_list_model_get_item(self->chan_list_model, i);
        if (!item){
            continue;
        }

        chan = sui_channel_list_item_get_channel(item);
        users = sui_channel_list_item_get_users(item);
        topic = sui_channel_list_item_get_topic(item);

        if (chan_item_visible(self, chan, users, topic)){
            gtk_list_box_insert(self->chan_list_box,
                    new_channel_row(chan, users, topic), -1);
        }
        g_object_unref(item);
    }
}

static const char *get_selected_channel(SuiJoinPanel *self){
    GtkListBoxRow *row;

    row = gtk_list_box_get_selected_row(self->chan_list_box);
    if (!row){
        return NULL;
    }

    return g_object_get_data(G_OBJECT(row), "channel");
}

static GtkWidget *new_channel_row(const char *chan, int users, const char *topic){
    GtkWidget *row;
    GtkWidget *box;
    GtkWidget *chan_label;
    GtkWidget *users_label;
    GtkWidget *topic_label;
    char *users_text;

    row = gtk_list_box_row_new();
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    chan_label = gtk_label_new(chan);
    users_text = g_strdup_printf("%d", users);
    users_label = gtk_label_new(users_text);
    topic_label = gtk_label_new(topic ? topic : "");

    gtk_widget_set_hexpand(chan_label, TRUE);
    gtk_widget_set_hexpand(topic_label, TRUE);
    gtk_label_set_xalign(GTK_LABEL(chan_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(users_label), 1.0);
    gtk_label_set_xalign(GTK_LABEL(topic_label), 0.0);
    gtk_label_set_ellipsize(GTK_LABEL(chan_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(topic_label), PANGO_ELLIPSIZE_END);

    gtk_box_append(GTK_BOX(box), chan_label);
    gtk_box_append(GTK_BOX(box), users_label);
    gtk_box_append(GTK_BOX(box), topic_label);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);

    g_object_set_data_full(G_OBJECT(row), "channel", g_strdup(chan), g_free);
    g_free(users_text);

    return row;
}
#endif
