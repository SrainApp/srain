/**
 * @file completion.c
 * @brief GtkEntryCompletion wrapper for srain
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-05
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "srain_entry_completion.h"

#include "log.h"

struct _SrainEntryCompletion {
    GtkEntryCompletion parent;
    GtkListStore *list;
};

struct _SrainEntryCompletionClass {
    GtkEntryCompletionClass parent_class;
};

G_DEFINE_TYPE(SrainEntryCompletion, srain_entry_completion, GTK_TYPE_ENTRY_COMPLETION);

static void srain_entry_completion_finalize(GObject *object){
    // if (SRAIN_ENTRY_COMPLETION(object)->list)
    // free(SRAIN_IMAGE(object)->list);

    G_OBJECT_CLASS(srain_entry_completion_parent_class)->finalize(object);
}

static void srain_entry_completion_init(SrainEntryCompletion *self){
    self->list = gtk_list_store_new(1, G_TYPE_STRING);
}

static void srain_entry_completion_class_init(SrainEntryCompletionClass *class){
    GObjectClass *object_class = G_OBJECT_CLASS (class);

    object_class->finalize = srain_entry_completion_finalize;
}

SrainEntryCompletion* srain_entry_completion_new(GtkEntry *entry){
    SrainEntryCompletion *comp;
    GtkEntryCompletion *comp2;

    comp = g_object_new(SRAIN_TYPE_ENTRY_COMPLETION, NULL);
    comp2 = GTK_ENTRY_COMPLETION(comp);

    gtk_entry_set_completion(entry, comp2);

    // gtk_entry_completion_set_inline_selection(comp2, TRUE);
    gtk_entry_completion_set_popup_completion(comp2, FALSE);
    gtk_entry_completion_set_popup_set_width(comp2, FALSE);
    gtk_entry_completion_set_popup_single_match(comp2, FALSE);

    /* Use a tree model as the comp model */
    gtk_entry_completion_set_model(comp2, GTK_TREE_MODEL(comp->list));
    gtk_entry_completion_complete(comp2);
    gtk_entry_completion_set_text_column(comp2, 0);

    return comp;
}

void srain_entry_completion_add_keyword(SrainEntryCompletion *comp, const char *keyword){
    GtkTreeIter iter;

    LOG_FR("keyword: '%s'", keyword);

    gtk_list_store_append(comp->list, &iter);
    gtk_list_store_set(comp->list, &iter, 0, keyword, -1);
}

void srain_entry_completion_rm_keyword(SrainEntryCompletion *comp, const char *keyword){
    LOG_FR("keyword: '%s'", keyword);
}

void srain_entry_completion_complete(SrainEntryCompletion *comp){
    int cur_pos;
    const char *word_ptr;
    const char *text;
    const char *word;
    const char *prefix;
    GtkEntry *entry;
    GtkEntryBuffer *buf;
    GtkEntryCompletion *comp2;

    comp2 = GTK_ENTRY_COMPLETION(comp);
    entry = GTK_ENTRY(gtk_entry_completion_get_entry(comp2));

    buf = gtk_entry_get_buffer(entry);
    text = gtk_entry_get_text(entry);

    cur_pos = gtk_editable_get_position(GTK_EDITABLE(entry));
    word_ptr = text + cur_pos;

    while (word_ptr > text){
        word_ptr = g_utf8_prev_char(word_ptr);
        if (*word_ptr == ' '){
            word_ptr++;
            break;
        }
    }

    // TODO: 中文处理有问题
    word = strndup(word_ptr, text + cur_pos - word_ptr);
    prefix = gtk_entry_completion_compute_prefix(comp2, word);

    if (prefix) {
        gtk_entry_buffer_insert_text(buf, cur_pos, prefix + strlen(word), -1);
        gtk_editable_set_position(GTK_EDITABLE(entry),
                cur_pos + strlen(prefix) - strlen(word));
        // gtk_editable_select_region(GTK_EDITABLE(entry),
        // cur_pos, cur_pos + strlen(prefix) - strlen(word));
    }

    LOG_FR("cur_pos: %d, word: '%s', prefix: '%s'", cur_pos, word, prefix);
}
