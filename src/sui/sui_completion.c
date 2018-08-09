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
 * @file sui_completion.c
 * @brief
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-06-05
 *
 * Ref: https://github.com/TingPing/irc-client/blob/master/src/irc-entrybuffer.c
 *
 */

#include <gtk/gtk.h>
#include <string.h>

#include "sui_completion.h"

#include "log.h"
#include "utils.h"

struct _SuiCompletion {
    GObject parent;

    GtkTextBuffer *text_buffer;

    /* Completion state */
    SuiCompletionFunc *last_func;
    void *last_user_data;
    char *last_prefix;  // Prefix of last completion
    char *last_suffix;  // Suffix of last completion

    GtkTreeModel *model;
    GtkTreeIter iter;
};

struct _SuiCompletionClass {
    GObjectClass parent_class;
};

static bool reset_state(SuiCompletion *self, SuiCompletionFunc *func,
        void *user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum
{
  // 0 for PROP_NOME
  PROP_TEXT_BUFFER = 1,
  N_PROPERTIES
};

G_DEFINE_TYPE(SuiCompletion, sui_completion, G_TYPE_OBJECT);

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void sui_completion_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec){
  SuiCompletion *self = SUI_COMPLETION(object);

  switch (property_id){
      case PROP_TEXT_BUFFER:
          sui_completion_set_text_buffer(self, g_value_get_object(value));
          break;
      default:
          /* We don't have any other property... */
          G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
          break;
  }
}

static void sui_completion_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec){
  SuiCompletion *self = SUI_COMPLETION(object);

    switch (property_id){
        case PROP_TEXT_BUFFER:
            g_value_set_object(value, sui_completion_get_text_buffer(self));
            break;
        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void sui_completion_init(SuiCompletion *self){
}

static void sui_completion_constructed(GObject *object){
    G_OBJECT_CLASS(sui_completion_parent_class)->constructed(object);
}

static void sui_completion_finalize(GObject *object){
    SuiCompletion *self;

    self = SUI_COMPLETION(object);

    sui_completion_set_text_buffer(self, NULL);

    str_assign(&self->last_prefix, NULL);
    str_assign(&self->last_suffix, NULL);

    if (self->model) {
        g_object_unref(self->model);
    }

    G_OBJECT_CLASS(sui_completion_parent_class)->finalize(object);
}

static void sui_completion_class_init(SuiCompletionClass *class){
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = sui_completion_constructed;
    object_class->finalize = sui_completion_finalize;
    object_class->set_property = sui_completion_set_property;
    object_class->get_property = sui_completion_get_property;

    /* Install properties */
    obj_properties[PROP_TEXT_BUFFER] =
        g_param_spec_object("text-buffer",
                "TextBuffer",
                "Completion target",
                GTK_TYPE_TEXT_BUFFER,
                G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class,
            N_PROPERTIES,
            obj_properties);
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiCompletion* sui_completion_new(GtkTextBuffer *text_buffer){
    return g_object_new(SUI_TYPE_COMPLETION,
            "text-buffer", text_buffer,
            NULL);
}

void sui_completion_complete(SuiCompletion *self, SuiCompletionFunc *func,
        void *user_data){
    int cursor_pos;
    GtkTextBuffer *buf;
    GtkTextIter start;
    GtkTextIter cursor;
    GtkTextIter comp;

    buf = self->text_buffer;
    g_return_if_fail(buf);

    /* If the completion function and user data changed, we should regenerate
     * completion list */
    if (func != self->last_func || user_data != self->last_user_data) {
        if (!reset_state(self, func, user_data)){
            return;
        }
    }

    g_object_get(buf, "cursor-position", &cursor_pos, NULL);
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_iter_at_offset(buf, &cursor, cursor_pos);
    gtk_text_buffer_get_iter_at_mark(buf, &comp,
            gtk_text_buffer_get_mark(buf, "completion-position"));

    { /* Check last completion prefix */
        char *ctx;

        ctx = gtk_text_buffer_get_text(buf, &start, &comp, FALSE);
        if (!g_str_has_suffix(ctx, self->last_prefix)){
            DBG_FR("Last prefix chagned: '%s' -> '%s'",
                    ctx, self->last_prefix);

            g_free(ctx);

            if (reset_state(self, func, user_data)){
                sui_completion_complete(self, func, user_data);
            }
            return;
        }
        g_free(ctx);
    }

    { /* Check last completion suffix */
        char *suffix;

        suffix = gtk_text_buffer_get_text(buf, &comp, &cursor, FALSE);
        if (g_ascii_strcasecmp(suffix, self->last_suffix) != 0){
            DBG_FR("Last suffix chagned: '%s' -> '%s'",
                    suffix, self->last_suffix);

            g_free(suffix);

            if (reset_state(self, func, user_data)){
                sui_completion_complete(self, func, user_data);
            }
            return;
        }
        g_free(suffix);
    }

    if (!self->model) {
        DBG_FR("Empty completion model");
        return;
    }

    str_assign(&self->last_prefix, NULL);
    str_assign(&self->last_suffix, NULL);
    gtk_tree_model_get(self->model, &self->iter,
            SUI_COMPLETION_COLUMN_PREFIX, &self->last_prefix,
            SUI_COMPLETION_COLUMN_SUFFIX, &self->last_suffix,
            -1);
    DBG_FR("Completion: prefix: '%s', suffix: '%s'",
            self->last_prefix, self->last_suffix);

    gtk_text_buffer_delete(buf, &comp, &cursor);
    gtk_text_buffer_insert(buf, &comp, self->last_suffix, strlen(self->last_suffix));

    if (!gtk_tree_model_iter_next(self->model, &self->iter)){
        if (!gtk_tree_model_get_iter_first(self->model, &self->iter)){
            DBG_FR("No next iterator");
            g_return_if_reached();
        }
    }
}

GtkTextBuffer* sui_completion_get_text_buffer(SuiCompletion *self){
    return self->text_buffer;
}

void sui_completion_set_text_buffer(SuiCompletion *self, GtkTextBuffer *text_buffer){
    if (self->text_buffer){
        g_object_unref(self->text_buffer);
    }
    self->text_buffer = g_object_ref(text_buffer);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static bool reset_state(SuiCompletion *self, SuiCompletionFunc *func,
        void *user_data){
    int cursor_pos;
    char *ctx;
    GtkTextBuffer *buf;
    GtkTextIter start;
    GtkTextIter cursor;

    DBG_FR("Reset completion state");

    buf = self->text_buffer;
    g_return_val_if_fail(buf, FALSE);

    g_object_get(buf, "cursor-position", &cursor_pos, NULL);
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_iter_at_offset(buf, &cursor, cursor_pos);

    gtk_text_buffer_create_mark(buf, "completion-position", &cursor, TRUE);

    self->last_func = func;
    self->last_user_data = user_data;
    str_assign(&self->last_prefix, "");
    str_assign(&self->last_suffix, "");

    if (self->model) {
        g_object_unref(self->model);
    }
    ctx = gtk_text_buffer_get_text(buf, &start, &cursor, FALSE);
    self->model = func(ctx, user_data); // Get completion list
    g_free(ctx);
    g_return_val_if_fail(self->model, FALSE);

    if (!gtk_tree_model_get_iter_first(self->model, &self->iter)){
        DBG_FR("Empty completion model");

        g_object_unref(self->model);
        self->model = NULL;

        return FALSE;
    }

    return TRUE;
}
