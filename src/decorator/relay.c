#define __DBG_ON
#define __LOG_ON

#include <glib.h>

#include "server.h"

#include "decorator.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "log.h"
#include "i18n.h"

static int relay(Message *msg, DecoratorFlag flag, void *user_data);

Decorator relay_decroator = {
    .name = "relay",
    .func = relay,
};

int relay_decroator_add_nick(Chat *chat, const char *nick){
    GSList *lst;

    lst = chat->relaybot_list;

    while(lst){
        if (sirc_nick_cmp(lst->data, nick)){
            chat_add_error_message_fmt(chat->srv->cur_chat, chat->user->nick,
                    _("\"%s\" already exists in %s 's relaybot list"),
                    nick, chat->name);
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    chat->relaybot_list = g_slist_append(chat->relaybot_list, g_strdup(nick));

    chat_add_misc_message_fmt(chat->srv->cur_chat, chat->user->nick,
            _("\"%s\" has added to %s 's relaybot list"), nick, chat->name);

    return SRN_OK;
}

int relay_decroator_rm_nick(Chat *chat, const char *nick){
    GSList *lst;

    lst = chat->relaybot_list;

    while(lst){
        if (lst->data){
            if (sirc_nick_cmp(lst->data, nick)){
                g_free(lst->data);
                chat->relaybot_list = g_slist_delete_link(chat->relaybot_list, lst);

                chat_add_misc_message_fmt(chat->srv->cur_chat, chat->user->nick,
                        _("\"%s\" is removed from %s 's relaybot list"),
                        nick, chat->name);

                return SRN_OK;
            }
        }
        lst = g_slist_next(lst);
    }

    chat_add_error_message_fmt(chat->srv->cur_chat, chat->user->nick,
            _("\"%s\" not found in %s 's relaybot list"),
            nick, chat->name);

    return SRN_ERR;
}

void relay_decroator_free_list(Chat *chat){
    g_slist_free_full(chat->relaybot_list, g_free);
    chat->relaybot_list = NULL;
}

static int relay(Message *msg, DecoratorFlag flag, void *user_data){
    int i;
    char *dnick;
    char *dcontent;
    GSList *lst;
    GError *err;
    GRegex *regex;
    GMatchInfo *match_info;

    /* ref: https://github.com/tuna/scripts/blob/master/weechat_bot2human.py#L46 */
    err = NULL;
    regex = g_regex_new("(\x03[0-9,]+)?\\[(?<nick>[^:]+?)\\]\x0f? (?<text>.*)", 0, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed: %s", err->message);
        return SRN_ERR;
    }

    for (i = 0, lst = msg->chat->relaybot_list;
            i < 2;
            i++, lst = msg->chat->srv->chat->relaybot_list){
        while (lst){
            if (sirc_nick_cmp(msg->user->nick, lst->data)){
                DBG_FR("Brige bot '%s' found", (char *)lst->data);
                g_regex_match(regex, msg->dcontent, 0, &match_info);

                if (g_match_info_matches(match_info)){
                    dnick = g_match_info_fetch_named(match_info, "nick");
                    dcontent = g_match_info_fetch_named(match_info, "text");

                    g_free(msg->dname);
                    g_free(msg->dcontent);
                    msg->dname = dnick;
                    msg->dcontent = dcontent;
                    msg->role = g_strdup(msg->user->nick);

                    DBG_FR("Match nick: %s", dnick);
                    DBG_FR("Match content: %s", dcontent);
                }

                g_match_info_free(match_info);

            }

            lst = g_slist_next(lst);
        }
    }

    g_regex_unref(regex);

    return SRN_OK;
}
