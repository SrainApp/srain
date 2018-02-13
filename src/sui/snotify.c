/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
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
 * @file snotify.c
 * @brief Libnotify based desktop notification
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-08-26
 */

#include <libnotify/notify.h>
#include <glib.h>

#include "meta.h"
#include "log.h"

#define SNOTIFY_TIMEOUT 5000

void snotify_init(){
    notify_init(PACKAGE_NAME);
}

void snotify_notify(const char *title, const char *msg, const char *icon){
    NotifyNotification *notify;

    notify = notify_notification_new (title, msg, icon);
    /* 3 seconds */
    notify_notification_set_timeout(notify, SNOTIFY_TIMEOUT);

    if (!notify_notification_show(notify, NULL)) {
        ERR_FR("Failed to send notification");
    }

    g_object_unref(G_OBJECT(notify));
}

void snotify_finalize(){
    notify_uninit();
}
