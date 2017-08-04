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

#include <string.h>
#include <glib.h>

#include "decorator.h"

#include "srain.h"
#include "log.h"

static int mirc_stirp(Message *msg, DecoratorFlag flag, void *user_data);

Decorator mirc_strip_decroator = {
    .name = "mirc_strip",
    .func = mirc_stirp,
};

int mirc_stirp(Message *msg, DecoratorFlag flag, void *user_data){
    int i;
    int j;
    int len;
    char *str;

    str = g_strdup(msg->dcontent);

    j = 0;
    len = strlen(str);

    for (i = 0; i < len; i++){
        switch (str[i]){
            case 2: case 0xf: case 0x16:
            case 0x1d: case 0x1f:
                break;
            case 3:  // irc color code
                if (str[i+1] >= '0' && str[i+1] <= '9'){
                    if (str[i+2] >= '0' && str[i+2] <= '9'){
                        i += 2;
                    } else {
                        i += 1;
                    }
                }
                break;
            default:
                str[j++] = str[i];
        }
    }

    str[j] = '\0';

    g_free(msg->dcontent);
    msg->dcontent = str;

    return SRN_OK;
}
