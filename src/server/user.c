#include "server.h"

User *user_new(Chat *chat, const char *nick, const char *username,
        const char *realname, UserType type){
    User *user;

    if (!username) username = "";
    if (!realname) realname = "";

    user = g_malloc0(sizeof(User));

    user->chat = chat;
    user->me = FALSE;
    user->type = type;

    g_strlcpy(user->nick, nick, sizeof(user->nick));
    g_strlcpy(user->username, username, sizeof(user->username));
    g_strlcpy(user->realname, realname, sizeof(user->realname));

    return user;
}

void user_free(User *user){
    g_free(user);
}

void user_rename(User *user, const char *new_nick){
    /* Update UI status */
    if (user->chat) {
        sui_ren_user(user->chat->ui, user->nick, new_nick, user->type);
    }

    g_strlcpy(user->nick, new_nick, sizeof(user->nick));

    return SRN_OK;
}

void user_set_type(User *user, UserType type){
    /* Update UI status */
    if (user->chat) {
        sui_ren_user(user->chat->ui, user->nick, user->nick, type);
    }

    user->type = type;

    return SRN_OK;
}
