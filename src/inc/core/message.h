/* Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <glib.h>

#ifndef __IN_CORE_H
	#error This file should not be included directly, include just core.h
#endif

typedef enum _SrnMessageType SrnMessageType;
typedef struct _SrnMessage SrnMessage;

#include "./chat.h"

enum _SrnMessageType {
    SRN_MESSAGE_TYPE_UNKNOWN,
    SRN_MESSAGE_TYPE_RECV,
    SRN_MESSAGE_TYPE_SENT,
    SRN_MESSAGE_TYPE_ACTION,
    SRN_MESSAGE_TYPE_NOTICE,
    SRN_MESSAGE_TYPE_MISC,
    SRN_MESSAGE_TYPE_ERROR,
};

struct _SrnMessage {
    SrnChat *chat;
    SrnChatUser *sender; // Sender of this message
    SrnMessageType type;

    /* Raw message */
    char *content;  // Raw message content
    GDateTime *time; // Local time when creating message

    /* NOTE: All rendered_xxx fields MUST be valid XML and never be NULL */
    char *rendered_sender; // Sender name
    char *rendered_remark; // Message remark
    char *rendered_content; // Rendered message content in
    char *rendered_short_time; // Short format message time
    char *rendered_full_time;  // Full format messsage time
    GList *urls; // URLs in message, like "http://xxx", "irc://xxx"

    bool mentioned; // Whether this message should be mentioned

    SuiMessage *ui;
};

SrnMessage* srn_message_new(SrnChat *chat, SrnChatUser *user, const char *content, SrnMessageType type);
void srn_message_free(SrnMessage *msg);
char* srn_message_to_string(SrnMessage *self);

#endif /* __MESSAGE_H */
