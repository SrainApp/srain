/**
 * @file completion.c
 * @brief GtkEntryCompletion wrapper for keywords completion
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-05
 *
 * Normal GtkEntryCompletion can only do completion according
 * to the full content of GtkEntry, SrainEntryCompletion can
 * do it according to the word at current cursor.
 *
 */

// #define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "srain_entry_completion.h"

#include "log.h"

#define TMP_QUEUE_LEN 50

struct _SrainEntryCompletion {
    GtkEntryCompletion parent;

    /* Queue for storing temporary keywords,
     * length not exceeding TMP_QUEUE_LEN.
     * If the limit is reached,
     * the last element will be removed  */
    GQueue *queue;
    /* Tree model for storing normal keywords */
    GtkListStore *list;
};

struct _SrainEntryCompletionClass {
    GtkEntryCompletionClass parent_class;
};

G_DEFINE_TYPE(SrainEntryCompletion, srain_entry_completion, GTK_TYPE_ENTRY_COMPLETION);

/**
 * @brief is_legal_keyword
 *
 * @param keyword
 *
 * @return if 1, legal, if 0, illegal
 *
 * Is `keyword` a legal keyword?
 */
static int is_legal_keyword(const char *keyword){
    while (*keyword){
        if (*keyword == '\t' && *keyword == ' '){
            ERR_FR("keyword: '%s' illegal", keyword);
            return 0;
        }
        keyword++;
    }
    return 1;
}

static const char* srain_entry_completion_compute_prefix(
        SrainEntryCompletion *comp, const char *key){
    const char *prefix;
    prefix = gtk_entry_completion_compute_prefix(
            GTK_ENTRY_COMPLETION(comp), key);

    if (prefix != NULL){
        return prefix;
    }

    // TODO: better match algorithm
    int i, len;

    len = g_queue_get_length(comp->queue);
    for (i = 0; i < len; i++){
        prefix = g_queue_peek_nth(comp->queue, i);
        if (strncmp(prefix, key, strlen(key)) == 0){
            LOG_FR("queue matched");
            return prefix;
        }
    }

    return NULL;
}

static void srain_entry_completion_finalize(GObject *object){
    GQueue *queue;

    /* SRAIN_ENTRY_COMPLETION(object)->list
     * will be free automatically */

    queue = SRAIN_ENTRY_COMPLETION(object)->queue;
    if (queue){
        while (!g_queue_is_empty(queue)){
            g_free(g_queue_pop_tail(queue));
        }
    }

    G_OBJECT_CLASS(srain_entry_completion_parent_class)->finalize(object);
}

static void srain_entry_completion_init(SrainEntryCompletion *self){
    self->queue = g_queue_new();
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
    g_object_unref(comp);

    gtk_entry_completion_set_inline_selection(comp2, FALSE);
    gtk_entry_completion_set_popup_completion(comp2, FALSE);
    gtk_entry_completion_set_popup_set_width(comp2, FALSE);
    gtk_entry_completion_set_popup_single_match(comp2, FALSE);

    /* Use a tree model as the comp model */
    gtk_entry_completion_set_model(comp2, GTK_TREE_MODEL(comp->list));
    g_object_unref(comp->list);

    /* (?) 少了这句的话 gtk_entry_completion_compute_prefix() 无法得出结果 */
    gtk_entry_completion_complete(comp2);
    gtk_entry_completion_set_text_column(comp2, 0);

    return comp;
}

/**
 * @brief srain_entry_completion_add_keyword
 *
 * @param comp
 * @param keyword
 * @param type If type = KEYWORD_TMP,
 *  `keyword` will be added to `comp`-> queue;
 *  If type == KEYWORD_NORMAL , `keyword` will
 *  be added to `comp`-> list.
 *
 * @return If 0, keyword added successfully.
 *
 * Append a whitespace to the end of `keyword`,
 * add this new string to the completion list of `comp`.
 */
int srain_entry_completion_add_keyword(SrainEntryCompletion *comp,
        const char *keyword, SECKeywordType type){
    LOG_FR("keyword: '%s', type: %d", keyword, type);

    if (!is_legal_keyword(keyword)){
        return -1;
    }

    if (type == KEYWORD_TMP){
        gpointer data;

        if (g_queue_get_length(comp->queue) > TMP_QUEUE_LEN){
            data = g_queue_pop_tail(comp->queue);
            g_free(data);
            LOG_FR("queue full");
        }

        g_queue_push_head(comp->queue, strdup(keyword));
    }
    else if (type == KEYWORD_NORMAL){
        GtkTreeIter iter;
        /* gtk_list_store_set: The value will be referenced by the store
         * if it is a G_TYPE_OBJECT,
         * and it will be copiedif it is a G_TYPE_STRING or G_TYPE_BOXED
         */
        gtk_list_store_append(comp->list, &iter);
        gtk_list_store_set(comp->list, &iter, 0, keyword, -1);
    } else {
        ERR_FR("Unsupported SECKeywordType: %d", type);
        return -1;
    }

    return 0;
}

/**
 * @brief srain_entry_completion_rm_keyword
 *
 * @param comp
 * @param keyword
 *
 * @return If 0, keyword removed successfully.
 *
 * Remove a whitespace appended keyword from `comp`->list.
 * For keywords in `comp`->queue, they will be removed
 * automatically when reach the limit.
 */
int srain_entry_completion_rm_keyword(SrainEntryCompletion *comp,
        const char *keyword){
    const char *val_str;
    GValue val = {0, };
    GtkTreeIter  iter;
    GtkTreeModel *tree_model;

    tree_model = GTK_TREE_MODEL(comp->list);

    if (!gtk_tree_model_get_iter_first(tree_model, &iter)){
        LOG_FR("empty");
        return -1;
    }

    do {
        gtk_tree_model_get_value(tree_model, &iter, 0, &val);
        val_str = g_value_get_string(&val);
        if (strcmp(keyword, val_str) == 0){
            gtk_list_store_remove(comp->list, &iter);
            return 0;
        }
        g_value_unset(&val);
    } while (gtk_tree_model_iter_next(tree_model, &iter));

    LOG_FR("not found");

    return -1;
}

void srain_entry_completion_complete(SrainEntryCompletion *comp){
    int cur_pos;
    int word_len;
    const char *word_ptr;
    const char *text;
    const char *word;
    const char *prefix;
    GtkEntry *entry;
    GtkEntryBuffer *buf;

    entry = GTK_ENTRY(gtk_entry_completion_get_entry(
                GTK_ENTRY_COMPLETION(comp)));

    buf = gtk_entry_get_buffer(entry);
    text = gtk_entry_get_text(entry);
    cur_pos = gtk_editable_get_position(GTK_EDITABLE(entry));

    int i = cur_pos;
    word_ptr = text;
    while (i){
        word_ptr = g_utf8_next_char(word_ptr);
        i--;
    }

    word_len = word_ptr - text;
    while (word_ptr > text){
        word_ptr = g_utf8_prev_char(word_ptr);
        if (*word_ptr == ' '){
            word_ptr++;
            break;
        }
    }

    word_len -= word_ptr - text;
    word = strndup(word_ptr, word_len);
    prefix = srain_entry_completion_compute_prefix(comp, word);

    if (prefix) {
        gtk_entry_buffer_insert_text(buf, cur_pos, prefix + word_len, -1);
        gtk_editable_set_position(GTK_EDITABLE(entry),
                cur_pos + strlen(prefix) - strlen(word));
        // gtk_editable_select_region(GTK_EDITABLE(entry),
        // cur_pos, cur_pos + strlen(prefix) - strlen(word));
    }

    LOG_FR("cur_pos: %d, word: '%s', word_len: %d, prefix: '%s'", cur_pos, word, word_len, prefix);
}
