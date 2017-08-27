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

#ifndef __FILTER_H
#define __FILTER_H

#include "srain.h"
#include "server.h"

#define FILTER_NICK         1 << 0
#define FILTER_REGEX        1 << 1
#define FILTER_CHAT_LOG     1 << 2

typedef int FilterFlag;

/**
 * @brief FilterFunc Any filter module should implement this function,
 *      accepts the plain text of XML message
 *
 * @param msg A Message instance, ``msg->dcontent`` should be valid XML which
 *      may without root tag
 * @param content The plain text of ``msg->dcontent``
 *
 * @return FALSE if filter thinks this message should be ignored
 */
typedef bool (FilterFunc) (const Message *msg, const char *content);

typedef struct _Filter {
    const char *name;
    FilterFunc *func;
} Filter;

void filter_init();
bool filter_message(const Message *msg, FilterFlag flag, void *user_data);

int nick_filter_add_nick(Chat *chat, const char *nick);
int nick_filter_rm_nick(Chat *chat, const char *nick);
void nick_filter_free_list(Chat *chat);

int regex_filter_add_pattern(Chat *chat, const char *name, const char *pattern);
int regex_filter_rm_pattern(Chat *chat, const char *name);
void regex_filter_free_list(Chat *chat);

#endif /* __FILTER_H */
