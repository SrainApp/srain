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

#ifndef __SUI_COMPLETION_H
#define __SUI_COMPLETION_H

#include <gtk/gtk.h>

#define SUI_TYPE_COMPLETION (sui_completion_get_type())
#define SUI_COMPLETION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_COMPLETION, SuiCompletion))
#define SUI_IS_COMPLETION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_COMPLETION))

enum {
    SUI_COMPLETION_COLUMN_PREFIX = 0,   // Prefix column of completion result
    SUI_COMPLETION_COLUMN_SUFFIX,       // Suffix column of completion result
    SUI_COMPLETION_N_COLUMNS,
};

/**
 * @brief SuiCompletionFunc returns a list of completion result according the
 * context
 *
 * @param context Content of SuiCompletion's text buffer
 * @param user_data User defined data
 *
 * @return Completion result, which is a multi-column list. See
 * SUI_COMPLETION_COL_RESULT and SUI_TYPE_COMPLETION
 */
typedef GtkTreeModel* (SuiCompletionFunc) (const char *context, void *user_data);

typedef struct _SuiCompletion SuiCompletion;
typedef struct _SuiCompletionClass SuiCompletionClass;

GType sui_completion_get_type(void);
SuiCompletion *sui_completion_new(GtkTextBuffer *buffer);
void sui_completion_complete(SuiCompletion *self, SuiCompletionFunc *func, void *user_data);

void sui_completion_set_text_buffer(SuiCompletion *self, GtkTextBuffer *text_buffer);
GtkTextBuffer* sui_completion_get_text_buffer(SuiCompletion *self);

#endif /* __SUI_COMPLETION_H */
