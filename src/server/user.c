// #define __DBG_ON

#include "server.h"
#include "log.h"

User *user_new(Chat *chat, const char *nick, const char *username,
        const char *realname, UserType type){
    User *user;

    g_return_val_if_fail(chat, NULL);
    g_return_val_if_fail(nick, NULL);
    if (!username) username = "";
    if (!realname) realname = "";

    user = g_malloc0(sizeof(User));

    user->chat = chat;
    user->me = FALSE;
    user->type = type;
    user->refcount = 1;

    g_strlcpy(user->nick, nick, sizeof(user->nick));
    g_strlcpy(user->username, username, sizeof(user->username));
    g_strlcpy(user->realname, realname, sizeof(user->realname));

    DBG_FR("User %p '%s' created", user, user->nick);

    return user;
}

User *user_ref(User *user){
    g_return_val_if_fail(user, NULL);

    user->refcount++;

    return user;
}

void user_free(User *user){
    g_return_if_fail(user);

    DBG_FR("User %p '%s', refcount: %d", user, user->nick, user->refcount);

    if (--user->refcount > 0){
        return;
    }

    DBG_FR("User %p '%s' freed", user, user->nick);

    g_free(user);
}

void user_rename(User *user, const char *new_nick){
    /* Update UI status */
    if (user->chat) {
        sui_ren_user(user->chat->ui, user->nick, new_nick, user->type);
    }

    g_strlcpy(user->nick, new_nick, sizeof(user->nick));
}

void user_set_type(User *user, UserType type){
    /* Update UI status */
    if (user->chat) {
        sui_ren_user(user->chat->ui, user->nick, user->nick, type);
    }

    user->type = type;
}
