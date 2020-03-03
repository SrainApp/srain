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

#ifndef __SUI_NOTIFICATION_H
#define  __SUI_NOTIFICATION_H

#include <gtk/gtk.h>

typedef struct _SuiNotification SuiNotification;

/**
 * @brief ``SuiNotification`` describes the content of a notification in SUI
 * module.
 */
struct _SuiNotification {
    char *id; // Notification ID, used to deduplicate and withdraw.
    char *icon;
    char *title;
    char *body;
};

SuiNotification* sui_notification_new(void);
void sui_notification_free(SuiNotification *self);

#endif /* __SUI_NOTIFICATION_H */
