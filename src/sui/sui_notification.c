/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
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
 * @file sui_notification.c
 * @brief 
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 
 * @date 2018-05-05
 */

#include "sui_notification.h"

#include "utils.h"

SuiNotification* sui_notification_new(void){
    SuiNotification *self;

    self = g_malloc0(sizeof(SuiNotification));

    return self;
}

void sui_notification_free(SuiNotification* self){
    str_assign(&self->id, NULL);
    str_assign(&self->icon, NULL);
    str_assign(&self->title, NULL);
    str_assign(&self->body, NULL);
    g_free(self);
}
