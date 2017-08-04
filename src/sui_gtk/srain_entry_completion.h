/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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

#ifndef __SRAIN_ENTRY_COMPLETION_H
#define __SRAIN_ENTRY_COMPLETION_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_ENTRY_COMPLETION (srain_entry_completion_get_type())
#define SRAIN_ENTRY_COMPLETION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_ENTRY_COMPLETION, SrainEntryCompletion))
#define SRAIN_IS_ENTRY_COMPLETION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_ENTRY_COMPLETION))

typedef enum {
    KEYWORD_NORMAL,
    KEYWORD_TMP,
} SECKeywordType;

typedef struct _SrainEntryCompletion SrainEntryCompletion;
typedef struct _SrainEntryCompletionClass SrainEntryCompletionClass;

GType srain_entry_completion_get_type(void);
SrainEntryCompletion *srain_entry_completion_new(GtkEntry *entry);
int srain_entry_completion_add_keyword(SrainEntryCompletion *comp, const char *keyword, SECKeywordType type);
int srain_entry_completion_rm_keyword(SrainEntryCompletion *comp, const char *keyword);
void srain_entry_completion_complete(SrainEntryCompletion *completion);

#endif /* __SRAIN_ENTRY_COMPLETION_H */
