#include "server.h"

#include "decorator.h"
#include "filter.h"
#include "chat_log.h"

#include "sirc/sirc.h"

extern SuiEvents ui_events;

Chat *chat_new(Server *srv, const char *name){
    bool ischan;
    Chat *chat;

    ischan = sirc_is_chan(name);
    chat = g_malloc0(sizeof(Chat));

    chat->joined = FALSE;
    chat->srv = srv;
    chat->user = srv->user;
    chat->user_list = NULL;
    chat->ui = sui_new_session(&ui_events,
            ischan ? SUI_SESSION_CHANNEL : SUI_SESSION_DIALOG);

    if (!chat->ui){
        goto bad;
    }

    g_strlcpy(chat->name, name, sizeof(chat->name));

    sui_set_ctx(chat->ui, chat);
    sui_start_session(chat->ui, name, srv->info->name);

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
    chat->user = NULL;
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


/* TODO: Append message in Chat->msg_list */
void chat_add_sent_message(Chat *chat, const char *content){
    User *user;
    Message *msg;
    DecoratorFlag dflag;

    user = chat->user;
    dflag = DECORATOR_PANGO_MARKUP;
    msg = message_new(chat, user, content);

    if (sirc_cmd_msg(chat->srv->irc, chat->name, content) == SRN_OK){
        if (decorate_message(msg, dflag, NULL) == SRN_OK){
            sui_add_sent_msg(chat->ui, msg->dcontent, 0);
        }
    } else {
        chat_add_error_message_fmt(chat, NULL, "Failed to send message \"%s\"",
                msg->content);
    }

    chat_log_fmt(chat->srv->info->name, chat->name, "<%s> %s", user->nick, content);

    message_free(msg);
}

void chat_add_recv_message(Chat *chat, User *user, const char *content){
    Message *msg;
    DecoratorFlag flag;

    flag = DECORATOR_PANGO_MARKUP | DECORATOR_RELAY | DECORATOR_MIRC_STRIP;;
    msg = message_new(chat, user, content);

    if (filter_message(msg, FILTER_NICK | FILTER_REGEX, NULL)){
        if (decorate_message(msg, flag, NULL) == SRN_OK){
            sui_add_recv_msg(chat->ui, msg->dname, msg->role, msg->dcontent,
                    msg->mentioned ? SRAIN_MSG_MENTIONED : 0);
        }

        chat_log_fmt(chat->srv->info->name, chat->name, "<%s> %s", user->nick, content);
    }

    message_free(msg);
}

void chat_add_notice_message(Chat *chat, User *user, const char *content){
    chat_add_recv_message(chat, user, content);
}

void chat_add_action_message(Chat *chat, User *user, const char *content){
    char *action_msg;
    Message *msg;
    DecoratorFlag dflag;

    dflag = DECORATOR_PANGO_MARKUP;
    msg = message_new(chat, user, content);

    if (filter_message(msg, FILTER_NICK | FILTER_REGEX, NULL)){
        if (user->me){
            if (sirc_cmd_action(chat->srv->irc, chat->name, content) != SRN_OK){
                // ...
            }
        } else {
            dflag |= DECORATOR_RELAY | DECORATOR_MIRC_STRIP;
        }

        if (decorate_message(msg, dflag, NULL) == SRN_OK){
            action_msg = g_strdup_printf("*** <b>%s</b> %s***",
                    msg->dname, msg->dcontent);

            sui_add_sys_msg(chat->ui, action_msg, SYS_MSG_ACTION,
                    msg->mentioned ? SRAIN_MSG_MENTIONED : 0);

            g_free(action_msg);
        }

        chat_log_fmt(chat->srv->info->name, chat->name, "* %s %s", user->nick, content);
    }

    message_free(msg);
}

void chat_add_misc_message(Chat *chat, User *user, const char *content){
    char *dcontent;
    DecoratorFlag dflag;

    dflag = DECORATOR_PANGO_MARKUP;
    dcontent = decorate_content(content, dflag);

    if (dcontent){
        sui_add_sys_msg(chat->ui, dcontent, SYS_MSG_NORMAL, 0);
        g_free(dcontent);
    }

    chat_log_fmt(chat->srv->info->name, chat->name, "- %s %s", user->nick, content);
}

void chat_add_misc_message_fmt(Chat *chat, User *user, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    chat_add_misc_message(chat, user, content);

    g_free(content);
}

void chat_add_error_message(Chat *chat, User *user, const char *content){
    char *dcontent;
    DecoratorFlag dflag;

    dflag = DECORATOR_PANGO_MARKUP;
    dcontent = decorate_content(content, dflag);

    if (dcontent){
        sui_add_sys_msg(chat->ui, dcontent, SYS_MSG_ERROR, 0);
        g_free(dcontent);
    }

    chat_log_fmt(chat->srv->info->name, chat->name, "! %s %s", user->nick, content);
}

void chat_add_error_message_fmt(Chat *chat, User *user, const char *fmt, ...){
    char *content;
    va_list args;

    va_start(args, fmt);
    content = g_strdup_vprintf(fmt, args);
    va_end(args);

    chat_add_error_message(chat, user, content);

    g_free(content);
}
