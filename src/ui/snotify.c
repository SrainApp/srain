/**
 * @file snotify.c
 * @brief Srain's desktop notification
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-08-26
 */

#define __DBG_ON
#define __LOG_ON

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
