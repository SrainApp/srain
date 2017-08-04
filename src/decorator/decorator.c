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

/**
 * @file decorator.c
 * @brief Decorator is a mechanism for processing XML fromatted message in
 *        flow style
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06
 * @date 2017-05-06
 */

#include <glib.h>
#include <string.h>

#include "decorator.h"

#include "srain.h"
#include "log.h"

#define MAX_DECORATOR   32  // Bits of a DecoratorFlag(int)

extern Decorator relay_decroator;
extern Decorator mirc_strip_decroator;
extern Decorator pango_markup_decroator;
extern Decorator mention_decroator;

static Decorator *decorators[MAX_DECORATOR];

void decorator_init(){
    memset(decorators, 0, sizeof(decorators));

    decorators[0] = &relay_decroator;
    decorators[2] = &mirc_strip_decroator;
    decorators[3] = &pango_markup_decroator;
    decorators[4] = &mention_decroator;
}

int decorate_message(Message *msg, DecoratorFlag flag, void *user_data){
    int ret;

    for (int i = 0; i < MAX_DECORATOR; i++){
        if ((flag & (1 << i))
                && decorators[i]
                && decorators[i]->name
                && decorators[i]->func){

            DBG_FR("Run decorator '%s' for message %p", decorators[i]->name, msg);

            ret = decorators[i]->func(msg, flag, user_data);
            if (ret != SRN_OK){
                ERR_FR("Decorator '%s' return %d for message %p",
                        decorators[i]->name, ret, msg);

                return ret;
            }
        } else {
            // DBG_FR("No available decorator for bit %d", i);
        }
    }

    return SRN_OK;
}

/*
char* decorate_content(const char *content, DecoratorFlag flag){
    char *dcontent;

    Message *msg = message_new(NULL, NULL, content);

    if (decorate_message(msg, flag, NULL) == SRN_OK){
        dcontent = g_strdup(msg->dcontent);
    } else {
        dcontent = NULL;
    }

    message_free(msg);

    return dcontent;
}
*/
