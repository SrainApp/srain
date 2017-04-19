#include <string.h>

#include "server.h"

#include "meta.h"
#include "decorator.h"
#include "filter.h"
#include "i18n.h"

#include "sirc/sirc.h"

extern SuiEvents ui_events;

static void append_image(Message *msg);

Chat *chat_new(Server *srv, const char *name){
    bool ischan;
    Chat *chat;
    SuiSessionFlag flag;

    g_return_val_if_fail(name, NULL);

    ischan = sirc_is_chan(name);
    chat = g_malloc0(sizeof(Chat));

    chat->joined = FALSE;
    chat->srv = srv;
    chat->user = user_ref(srv->user);
    chat->user_list = NULL;

    flag = ischan ? SUI_SESSION_CHANNEL : SUI_SESSION_DIALOG;
    if (strcmp(META_SERVER, name) == 0){
        flag = SUI_SESSION_SERVER;
    }

    chat->ui = sui_new_session(&ui_events, flag);

    if (!chat->ui){
        goto cleanup;
    }

    g_strlcpy(chat->name, name, sizeof(chat->name));

    sui_set_ctx(chat->ui, chat);
    sui_start_session(chat->ui, name, srv->info->name);

    /* Require chat->ui */
    chat_add_user_full(chat, chat->user);
    if (flag == SUI_SESSION_DIALOG){
        chat_add_user(chat, chat->name, USER_CHIGUA);
    }

    return chat;

cleanup:
    if (chat->ui) {
        sui_free_session(chat->ui);
    }
    if (chat){
        g_free(chat);
    }

    return NULL;
}

void chat_free(Chat *chat){
    user_free(chat->user);

    /* Free extra list */
    if (chat->relaybot_list){
        relay_decroator_free_list(chat);
    }

    if (chat->ignore_nick_list){
        nick_filter_free_list(chat);
    }

    if (chat->ignore_regex_list){
        regex_filter_free_list(chat);
    }

    /* Free user list */
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
        chat->ui = NULL;
    }

    g_free(chat);
}

int chat_add_user(Chat *chat, const char *nick, UserType type){
    int ret;
    User *user;


    user = user_new(chat, nick, NULL, NULL, type);
    ret = chat_add_user_full(chat, user);
    user_free(user);

    return ret;
}

int chat_add_user_full(Chat *chat, User *user){
    GSList *lst;
    User *user2;

    lst = chat->user_list;
    while (lst){
        user2 = lst->data;
        if (sirc_nick_cmp(user2->nick, user->nick)){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    g_return_val_if_fail(user, SRN_ERR);

    chat->user_list = g_slist_append(chat->user_list, user_ref(user));
    sui_add_user(chat->ui, user->nick, user->type);

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


/* TODO: Append message in Chat->msg_list */
void chat_add_sent_message(Chat *chat, const char *content){
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat->user;
    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_CHAT_LOG;
    msg = message_new(chat, user, content, MESSAGE_SENT);

    if (sirc_cmd_msg(chat->srv->irc, chat->name, content) != SRN_OK){
        chat_add_error_message_fmt(chat, chat->user->nick,
                _("Failed to send message \"%s\""), msg->content);
        goto cleanup;
    }

    if (!filter_message(msg, fflag, NULL)){
        /* Ignore this message */
        goto cleanup;
    }

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }

    msg->ui = sui_add_sent_msg(chat->ui, msg->dcontent, 0);
    if (!msg->ui){
        goto cleanup;
    }

    append_image(msg);

    chat->msg_list = g_list_append(chat->msg_list, msg);
    return;

cleanup:
    message_free(msg);
}

void chat_add_recv_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    dflag = DECORATOR_PANGO_MARKUP |DECORATOR_RELAY | DECORATOR_MIRC_STRIP | DECORATOR_MENTION;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    msg = message_new(chat, user, content, MESSAGE_RECV);

    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }

    msg->ui = sui_add_recv_msg(chat->ui, msg->dname, msg->role, msg->dcontent,
            msg->mentioned ? SRAIN_MSG_MENTIONED : 0);
    if (!msg->ui){
        goto cleanup;
    }

    append_image(msg);

    chat->msg_list = g_list_append(chat->msg_list, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_notice_message(Chat *chat, const char *origin, const char *content){
    chat_add_recv_message(chat, origin, content);
}

void chat_add_action_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    FilterFlag fflag;
    DecoratorFlag dflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_CHAT_LOG;

    msg = message_new(chat, user, content, MESSAGE_ACTION);

    if (user->me){
        if (sirc_cmd_action(chat->srv->irc, chat->name, content) != SRN_OK){
            chat_add_error_message_fmt(chat, user->nick,
                    _("Failed to send action message \"%s\""),
                    msg->content);
            goto cleanup;
        }
    } else {
        fflag |= FILTER_NICK | FILTER_REGEX;
        dflag |= DECORATOR_RELAY | DECORATOR_MIRC_STRIP | DECORATOR_MENTION;
    }

    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }
    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }

    {
        // TODO: "<b>" no used?
        char *action_msg = g_strdup_printf(_("*** <b>%s</b> %s***"),
                msg->dname, msg->dcontent);

        msg->ui = sui_add_sys_msg(chat->ui, action_msg, SYS_MSG_ACTION,
                msg->mentioned ? SRAIN_MSG_MENTIONED : 0);
        g_free(action_msg);
    }


    if (!msg->ui){
        goto cleanup;
    }

    append_image(msg);

    chat->msg_list = g_list_append(chat->msg_list, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_misc_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = message_new(chat, user, content, MESSAGE_MISC);

    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }

    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }

    msg->ui = sui_add_sys_msg(chat->ui, msg->dcontent, SYS_MSG_NORMAL, 0);
    if (!msg->ui){
        goto cleanup;
    }

    chat->msg_list = g_list_append(chat->msg_list, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_misc_message_fmt(Chat *chat, const char *origin, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    chat_add_misc_message(chat, origin, content);

    g_free(content);
}

void chat_add_error_message(Chat *chat, const char *origin, const char *content){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;
    FilterFlag fflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_PANGO_MARKUP;
    fflag = FILTER_NICK | FILTER_REGEX | FILTER_CHAT_LOG;
    msg = message_new(chat, user, content, MESSAGE_ERROR);

    if (!filter_message(msg, fflag, NULL)){
        goto cleanup;
    }
    if (decorate_message(msg, dflag, NULL) != SRN_OK){
        goto cleanup;
    }

    msg->ui = sui_add_sys_msg(chat->ui, msg->dcontent, SYS_MSG_ERROR, 0);
    if (msg->ui){
        goto cleanup;
    }

    chat->msg_list = g_list_append(chat->msg_list, msg);
    return;

cleanup:
    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

void chat_add_error_message_fmt(Chat *chat, const char *origin, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    chat_add_error_message(chat, origin, content);

    g_free(content);
}

void chat_set_topic(Chat *chat, const char *origin, const char *topic){
    bool invalid_user = FALSE;
    User *user;
    Message *msg;
    DecoratorFlag dflag;

    user = chat_get_user(chat, origin);
    if (!user){
        user = user_new(chat, origin, NULL, NULL, USER_CHIGUA);
        invalid_user = TRUE;
    }

    dflag = DECORATOR_MIRC_STRIP | DECORATOR_PANGO_MARKUP;
    msg = message_new(chat, user, topic, MESSAGE_UNKNOWN);

    if (decorate_message(msg, dflag, NULL) == SRN_OK){
        sui_set_topic(chat->ui, msg->dcontent);
    }

    if (invalid_user){
        user_free(user);
    }
    message_free(msg);
}

// TODO
static void append_image(Message *msg){
    GSList *url;

    url = msg->urls;

    while (url){
        if (g_str_has_prefix(url->data, "http") // Both "http" and "https"
                && (g_str_has_suffix(url->data, "png")
                    || g_str_has_suffix(url->data, "jpg")
                    || g_str_has_suffix(url->data, "jpeg")
                    || g_str_has_suffix(url->data, "bmp")
                    || g_str_has_suffix(url->data, "gif")
                    )){
            sui_message_append_image(msg->ui, url->data);
        }
        url = g_slist_next(url);
    }
}
