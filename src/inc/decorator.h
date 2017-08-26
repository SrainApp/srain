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

#ifndef __DECORATOR_H
#define __DECORATOR_H

#include "server.h"

#define DECORATOR_RELAY         1 << 0
#define DECORATOR_MIRC_COLORIEZ 1 << 1
#define DECORATOR_MIRC_STRIP    1 << 2
#define DECORATOR_PANGO_MARKUP  1 << 3
#define DECORATOR_MENTION       1 << 4

typedef int DecoratorFlag;

/**
 * @brief DecoratorFunc Decorator module will parse the ``dcontent`` of Message,
 *      Divide it text by xml into several fragments. And pass the Message,
 *      fragment, the index of fragment to this function.
 *
 * @param msg A Message instance
 * @param index The index of ``frag`` in the current ``msg``
 * @param frag Fragment of ``msg->dcontent``, doesn't contain any XML tags
 *
 * @return The decorated fragment, should be freed by ``g_free()``
 */
typedef char* (DecoratorFunc) (Message *msg, int index, const char *frag);

typedef struct _Decorator {
    const char *name;
    DecoratorFunc *func;
} Decorator;

void decorator_init();
SrnRet decorate_message(Message *msg, DecoratorFlag flag, void *user_data);

int relay_decroator_add_nick(Chat *chat, const char *nick);
int relay_decroator_rm_nick(Chat *chat, const char *nick);
void relay_decroator_free_list(Chat *chat);

#endif /* __DECORATOR_H */
