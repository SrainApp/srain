#include <glib.h>

#include "server.h"

#include "srain.h"

Message* message_new(Chat *chat, User *user, const char *content){
    Message *msg;

    g_return_val_if_fail(content, NULL);

    msg = g_malloc0(sizeof(Message));

    msg->user = user;
    msg->chat = chat;
    // msg->role = NULL; // via g_malloc0()
    msg->content = g_strdup(content);
    // msg->time = ...;
    msg->mentioned = FALSE;
    // msg->urls = NULL; // via g_malloc0()

    /* Decorated */
    msg->dname = user ? g_strdup(user->nick) : NULL;
    msg->dcontent = g_strdup(content);

    return msg;
}

void message_free(Message *msg){
    g_return_if_fail(msg);

    if (msg->chat) { /* Nothing to do. */ }
    if (msg->user) { /* Nothing to do. */ }

    if (msg->urls) {
        GSList *lst = msg->urls;
        while (lst){
            g_free(lst->data);
            lst->data = NULL;
            lst = g_slist_next(lst);
        }
        g_slist_free(msg->urls);
    }

    if (msg->dname) {
        g_free(msg->dname);
    }

    if (msg->role) {
        g_free(msg->role);
    }

    if (msg->content) {
        g_free(msg->content);
    }

    if (msg->dcontent) {
        g_free(msg->dcontent);
    }

    g_free(msg);
}