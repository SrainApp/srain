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

#ifndef __SRAIN_BUFFER_H
#define __SRAIN_BUFFER_H

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "srain_msg.h"
#include "srain_msg_list.h"
#include "srain_user_list.h"
#include "srain_entry_completion.h"

#define SRAIN_TYPE_BUFFER (srain_buffer_get_type())
#define SRAIN_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_BUFFER, SrainBuffer))
#define SRAIN_IS_BUFFER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_BUFFER))

struct _SrainBuffer {
    GtkBox parent;

    SuiSession *session;
    char *name;
    char *remark;

    /* Header */
    GtkLabel* name_label;
    GtkLabel* remark_label;
    GtkRevealer *topic_revealer;
    GtkLabel *topic_label;

    /* Menus */
    GtkMenu *menu;
    GtkCheckMenuItem *topic_menu_item;

    /* Message list */
    GtkBox *msg_list_box;
    SrainMsgList *msg_list;

    /* User list container, used in sub class SrainChatBuffer
     * TODO: move to SrainChatBuffer */
    GtkRevealer *user_list_revealer;

    /* Input entry */
    GtkLabel *nick_label;
    GtkEntry *input_entry;
    SrainEntryCompletion *completion;
    GtkButton *upload_image_button;
};

struct _SrainBufferClass {
    GtkBoxClass parent_class;
};

typedef struct _SrainBuffer SrainBuffer;
typedef struct _SrainBufferClass SrainBufferClass;

GType srain_buffer_get_type(void);
SrainBuffer* srain_buffer_new(SuiSession *sui, const char *name, const char *remark);

void srain_buffer_fcous_entry(SrainBuffer *buffer);
void srain_buffer_insert_text(SrainBuffer *buffer, const char *text, int pos);
void srain_buffer_show_topic(SrainBuffer *buffer, bool isshow);

void srain_buffer_set_name(SrainBuffer *buffer, const char *name);
const char* srain_buffer_get_name(SrainBuffer *buffer);
void srain_buffer_set_remark(SrainBuffer *buffer, const char *remark);
const char* srain_buffer_get_remark(SrainBuffer *buffer);
void srain_buffer_set_nick(SrainBuffer *buffer, const char *nick);
const char* srain_buffer_get_nick(SrainBuffer *buffer);
void srain_buffer_set_topic(SrainBuffer *buffer, const char *topic);

GtkMenu* srain_buffer_get_menu(SrainBuffer *buffer);
SrainMsgList* srain_buffer_get_msg_list(SrainBuffer *buffer);
SrainEntryCompletion* srain_buffer_get_entry_completion(SrainBuffer *buffer);
void srain_buffer_set_session(SrainBuffer *buffer, SuiSession *session);
SuiSession *srain_buffer_get_session(SrainBuffer *buffer);

#endif /* __SRAIN_BUFFER_H */
