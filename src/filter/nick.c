#include <string.h>
#include <glib.h>

#include "sirc/sirc.h"

#include "server.h"

#include "filter.h"

#include "srain.h"
#include "log.h"
#include "i18n.h"

static bool nick(const Message *msg, FilterFlag flag, void *user_data);

Filter nick_filter = {
    .name = "nick",
    .func = nick,
};

int nick_filter_add_nick(Chat *chat, const char *nick){
    GSList *lst; 

    lst = chat->ignore_nick_list;

    while(lst){
        if (lst->data){
            if (sirc_nick_cmp(lst->data, nick)){
                chat_add_error_message_fmt(chat, NULL,
                        _("\"%s\" already exists in %s 's ignore list"),
                        nick, chat->name);
                return SRN_ERR;
            }
        }
        lst = g_slist_next(lst);
    }

    chat->ignore_nick_list = g_slist_append(chat->ignore_nick_list, g_strdup(nick));

    chat_add_misc_message_fmt(chat, NULL,
            _("\"%s\" has added to %s 's ignore list"), nick, chat->name);

    return SRN_OK;
}

int nick_filter_rm_nick(Chat *chat, const char *nick){
    GSList *lst; 

    lst = chat->ignore_nick_list;

    while(lst){
        if (lst->data){
            if (sirc_nick_cmp(lst->data, nick)){
                g_free(lst->data);
                chat->ignore_nick_list = g_slist_delete_link(chat->ignore_nick_list, lst);

                chat_add_misc_message_fmt(chat, NULL,
                        _("\"%s\" is removed from %s 's ignore list"),
                        nick, chat->name); 

                return SRN_OK;
            }
        }
        lst = g_slist_next(lst);
    }

    chat_add_error_message_fmt(chat, NULL,
            _("\"%s\" not found in %s 's ignore list"),
            nick, chat->name); 

    return SRN_ERR;
}

void nick_filter_free(Chat *chat, const char *nick){
    g_slist_free_full(chat->ignore_nick_list, g_free);
    chat->ignore_nick_list = NULL;
}

bool nick(const Message *msg, FilterFlag flag, void *user_data){
    GSList *lst;

    g_return_val_if_fail(msg->user, TRUE);
    g_return_val_if_fail(msg->chat, TRUE);
    g_return_val_if_fail(msg->chat->srv, TRUE);

    lst = msg->chat->ignore_nick_list;

    while (lst){
        if (sirc_nick_cmp(lst->data, msg->user->nick)){
            return FALSE;
        }
        lst = g_slist_next(lst);
    }

    return TRUE;
}
