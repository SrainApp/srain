/* Copyright (C) 2016-2020 Shengyu Zhang <i@silverrainz.me>
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

#include <glib.h>
#include <string.h>

#include "sirc_cmd_builder.h"

#define SIRC_RFC_CRLF "\r\n"
#define SIRC_RFC_PARAM_DELIM " "
#define SIRC_RFC_TRAILING_PREFIX ":"

// The "prefix" means a servername or a nick!user@host in IRC message
//
// NOTE: We knowns that IRC message has a 512 bytes limit, even we sent a
// 512-bytes IRC command to server, it still may be truncated by server when
// server forward your command to your message destination.
//
// For example: You send command "PRIVMSG #archlinux-cn :BlahBlah..." whose
// length is EXACTLY 512 bytes. When server forwards this command to everyone
// in channel, it should add your user info as prefix of message, so it sent send:
// ":nick!~user@hostname PRIVMSG #archlinux-cn :BlahBlah..." whose length MUST
// exceeds 512 bytes.
//
// It is hard to known length of user's prefix(a.k.a "nick!~user@hostname"),
// so we have to use such an magic number:
#define MAGIC_PREFIX_LEN 50
#define SIRC_RFC_MESSAGE_SZIE (512 - MAGIC_PREFIX_LEN)

struct _SircCommandBuilder {
    char *cmd;
    GList *middles; // List of middle param
    char *trailing; // Trailing param
    int expected_len; // Expected command length
};

/**
 * @brief Create a SircCommandBuilder instance.
 *
 * @param cmd
 *
 * @return
 */
SircCommandBuilder* sirc_command_builder_new(const char *cmd) {
    SircCommandBuilder *self;

    self = g_malloc0(sizeof(SircCommandBuilder));
    self->cmd = g_strdup(cmd);
    self->expected_len = strlen(cmd) + strlen(SIRC_RFC_CRLF);

    return self;
}

/**
 * @brief Free a SircCommandBuilder instance.
 *
 * @param self
 */
void sirc_command_builder_free(SircCommandBuilder *self) {
    g_free(self->cmd);
    g_list_free_full(self->middles, g_free);
    g_free(self->trailing);
    g_free(self);
}

/**
 * @brief Add a middle param to builder.
 *
 * @param self
 * @param param
 *
 * @return TRUE if the param is successfully added;
 * FALSE if length of command exceeds SIRC_RFC_MESSAGE_SZIE after param added,
 * nothing added.
 */
bool sirc_command_builder_add_middle(SircCommandBuilder *self, const char *param) {
    int added_len;

    added_len = strlen(SIRC_RFC_PARAM_DELIM) + strlen(param);
    if (self->expected_len + added_len > SIRC_RFC_MESSAGE_SZIE) {
        // Message length exceeds max message size, don't accpet param
        return FALSE;
    }

    self->middles = g_list_append(self->middles, g_strdup(param));
    self->expected_len += added_len;
    return TRUE;
}

/**
 * @brief Add a trailing param to builder.
 *
 * @param self
 * @param param
 *
 * @return remaining param that failed to add to builder due to length
 * limitation(SIRC_RFC_MESSAGE_SZIE). If whole param added, NULL is returned.
 *
 * UTF-8 param can be handled correctly.
 */
const char* sirc_command_builder_set_trailing(SircCommandBuilder *self, const char *param) {
    // Repeat set is not allwoed
    g_warn_if_fail(!self->trailing);

    int added_len = strlen(SIRC_RFC_PARAM_DELIM) + strlen(SIRC_RFC_TRAILING_PREFIX) + strlen(param);
    if (self->expected_len + added_len > SIRC_RFC_MESSAGE_SZIE) {
        // Message length exceeds max message size, try truncate param
        // NOTE: Please correctly handle UTF-8 chars
        int exceeded_len = self->expected_len + added_len - SIRC_RFC_MESSAGE_SZIE;
        const char *end = param + strlen(param);
        const char *prev = param+ strlen(param);
        for (;;) {
            prev = g_utf8_find_prev_char(param, prev);
            if (!prev) {
                // Can not truncate
                return param;
            }
            if (end - prev > exceeded_len) {
                // Found that [param, prev) can just add to the command without
                // exceeding the limit
                break;
            }
        }
        self->trailing = g_strndup(param, prev - param);
        self->expected_len += strlen(SIRC_RFC_PARAM_DELIM) + strlen(SIRC_RFC_TRAILING_PREFIX) + (prev - param);
        return prev;
    }

    self->trailing = g_strdup(param);
    self->expected_len += added_len;
    return NULL;
}


/**
 * @brief Build a legal length IRC message from builder.
 *
 * @param self
 *
 * @return a newly-allocated string, must be freed by g_free().
 */
char* sirc_command_builder_build(SircCommandBuilder *self) {
    GString *buf = g_string_new(self->cmd);
    for (GList *elem = self->middles; elem != NULL; elem = g_list_next(elem)) {
        const char *middle = elem->data;
        g_string_append(buf, SIRC_RFC_PARAM_DELIM);
        g_string_append(buf, middle);
    }
    if (self->trailing) {
        g_string_append(buf, SIRC_RFC_PARAM_DELIM);
        g_string_append(buf, SIRC_RFC_TRAILING_PREFIX);
        g_string_append(buf, self->trailing);
    }
    g_string_append(buf, SIRC_RFC_CRLF);

    g_warn_if_fail(buf->len == self->expected_len);
    char *cmd = buf->str;
    g_string_free(buf, FALSE);
    return cmd;
}
