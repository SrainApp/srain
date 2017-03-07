#include "server.h"

#include "sirc/sirc.h"

extern SuiEvents ui_events;

Chat *chat_new(Server *srv, const char *name){
    bool ischan;
    Chat *chat;

    ischan = sirc_is_chan(name);
    chat = g_malloc0(sizeof(Chat));

    chat->joined = FALSE;
    chat->srv = srv;
    chat->me = NULL;
    chat->user_list = NULL;
    chat->ui = sui_new_session(&ui_events,
            ischan ? SUI_SESSION_CHANNEL : SUI_SESSION_DIALOG);

    if (!chat->ui){
        goto bad;
    }

    g_strlcpy(chat->name, name, sizeof(chat->name));

    sui_set_ctx(chat->ui, chat);
    sui_start_session(chat->ui, name, srv->name);

    return chat;

bad:
    if (chat->ui) {
        sui_free_session(chat->ui);
    }
    if (chat){
        g_free(chat);
    }

    return NULL;
}

void chat_free(Chat *chat){
    /* Free user list */
    chat->me = NULL;
    if (chat->user_list){
        GSList *lst = chat->user_list;
        while (lst){
            if (lst->data){
                user_free((User *)lst->data);
                lst->data = NULL;
            }
            lst = g_slist_next(lst);
        }
        g_slist_free(chat->user_list);
        chat->user_list = NULL;
    }

    if (chat->ui){
        sui_free_session(chat->ui);
    }

    g_free(chat);
}

int chat_add_user(Chat *chat, const char *nick, UserType type){
    GSList *lst;
    User *user;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (sirc_nick_cmp(user->nick, nick)){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    user = user_new(chat, nick, NULL, NULL, type);
    g_return_val_if_fail(user, SRN_ERR);

    chat->user_list = g_slist_append(chat->user_list, user);
    sui_add_user(chat->ui, nick, type);

    return SRN_OK;
}

int chat_rm_user(Chat *chat, const char *nick){
    GSList *lst;
    User *user;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (sirc_nick_cmp(user->nick, nick)){
            user_free(user);
            sui_rm_user(chat->ui, user->nick);
            chat->user_list = g_slist_delete_link(chat->user_list, lst);

            return SRN_OK;
        }
        lst = g_slist_next(lst);
    }
    return SRN_ERR;
}


User* chat_get_user(Chat *chat, const char *nick){
    User *user;
    GSList *lst;

    lst = chat->user_list;
    while (lst){
        user = lst->data;
        if (sirc_nick_cmp(user->nick, nick)){
            return user;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}
