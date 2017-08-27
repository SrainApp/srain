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

#define DECORATOR_MIRC_STRIP    1 << 0
#define DECORATOR_MIRC_COLORIZE 1 << 1
#define DECORATOR_RELAY         1 << 2
#define DECORATOR_PANGO_MARKUP  1 << 3
#define DECORATOR_MENTION       1 << 4

typedef int DecoratorFlag;

/**
 * @brief DecoratorFunc Any decorator module should implement this function,
 *      accepts the text fragment of XML message, decorates it then return
 *      the decorated fragment.
 *
 * @param msg A Message instance, ``msg->dcontent`` should be valid XML which
 *      may without root tag
 * @param index The index of ``frag`` in the current ``msg``
 * @param frag Plain text fragment of ``msg->dcontent``
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
