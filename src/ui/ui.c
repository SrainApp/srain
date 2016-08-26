/* @file ui.c
 * @brief UI module interfaces
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-29
 */

// #define __DBG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "ui.h"
#include "ui_hdr.h"
#include "srain_app.h"
#include "srain_chat.h"
#include "srain_window.h"
#include "theme.h"
#include "snotify.h"

#include "srv_session.h"

#include "i18n.h"
#include "log.h"
#include "meta.h"

typedef struct {
    void *ui_interface;
    char srv_name[HOST_LEN];
    char chat_name[CHAN_LEN];
    char nick[NICK_LEN];
    char nick2[NICK_LEN];
    char msg[MSG_LEN];
    int type;
} CommonUIData;

void ui_init(int argc, char **argv){
    ui_hdr_init();
    theme_init();
    snotify_init();

    g_application_run(G_APPLICATION(srain_app_new()), argc, argv);

    snotify_finalize();
}

void ui_idle_destroy_data(void *data){
    DBG_FR("CommonUIData %p freed", data);
    g_free(data);
}

int ui_idle(CommonUIData *data){
    DBG_FR("Idle call, data: %p", data);
    DBG_FR("func: %p", data->ui_interface);

    if (data->ui_interface == ui_add_chat_sync){
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *nick = data->nick;
        ChatType type = data->type;
        ui_add_chat_sync(srv_name, chat_name, nick, type);
    }
    else if (data->ui_interface == ui_rm_chat_sync){
        DBG_FR("ui_rm_chat_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        ui_rm_chat_sync(srv_name, chat_name);
    }
    else if (data->ui_interface == ui_sys_msg_sync){
        DBG_FR("ui_sys_msg_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *msg = data->msg;
        SysMsgType type = data->type;
        ui_sys_msg_sync(srv_name, chat_name, msg, type);
    }
    else if (data->ui_interface == ui_send_msg_sync){
        DBG_FR("ui_send_msg_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *msg = data->msg;
        ui_send_msg_sync(srv_name, chat_name, msg);
    }
    else if (data->ui_interface == ui_recv_msg_sync){
        DBG_FR("ui_recv_msg_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *nick = data->nick;
        const char *id = data->nick2;
        const char *msg = data->msg;
        ui_recv_msg_sync(srv_name, chat_name, nick, id, msg);
    }
    else if (data->ui_interface == ui_add_user_sync){
        DBG_FR("ui_add_user_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *nick = data->nick;
        UserType type = data->type;
        ui_add_user_sync(srv_name, chat_name, nick, type);
    }
    else if (data->ui_interface == ui_rm_user_sync){
        DBG_FR("ui_rm_user_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *nick = data->nick;
        ui_rm_user_sync(srv_name, chat_name, nick);
    }
    else if (data->ui_interface == ui_ren_user_sync){
        DBG_FR("ui_ren_user_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *nick = data->nick;
        const char *new_nick = data->nick2;
        UserType type = data->type;
        ui_ren_user_sync(srv_name, chat_name, nick, new_nick, type);
    }
    else if (data->ui_interface == ui_set_topic_sync){
        DBG_FR("ui_set_topic_sync");
        const char *srv_name = data->srv_name;
        const char *chat_name = data->chat_name;
        const char *topic = data->msg;
        ui_set_topic_sync(srv_name, chat_name, topic);
    }
    else {
        ERR_FR("Invaild function pointer :(");
    }

    return FALSE;
}

/* =======================================================================
 * Note: The following functions are asynchronous and thread-safed, enjoy~
 * Arguments can be NULL, and will convert to ""(empty string) when passing
 * the data to main thread.
 * =======================================================================
 */

/*
#define CHECK_IF_NULL(x) \
    do { \
        if (!x) x = ""; \
        WARN_FR("`"#x "` is NULL, convert to empty string"); \
    } while (0)
*/

void ui_add_chat(const char *srv_name, const char *chat_name,
        const char *nick, ChatType type){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(nick);

    data->ui_interface = ui_add_chat_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->nick, nick, sizeof(data->nick));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_rm_chat(const char *srv_name, const char *chat_name){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);

    data->ui_interface = ui_rm_chat_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_sys_msg(const char *srv_name, const char *chat_name,
        const char *msg, SysMsgType type){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(msg);

    data->ui_interface = ui_sys_msg_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->msg, msg, sizeof(data->msg));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_send_msg(const char *srv_name, const char *chat_name,
        const char *msg){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(msg);

    data->ui_interface = ui_send_msg_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->msg, msg, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_recv_msg(const char *srv_name, const char *chat_name,
        const char *nick, const char *id, const char *msg){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(nick);
    g_return_if_fail(msg);
    g_return_if_fail(id);

    data->ui_interface = ui_recv_msg_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->nick, nick, sizeof(data->nick));
    strncpy(data->nick2, id, sizeof(data->msg));
    strncpy(data->msg, msg, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_add_user(const char *srv_name, const char *chat_name,
 const char *nick, UserType type){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(nick);

    data->ui_interface = ui_add_user_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->nick, nick, sizeof(data->nick));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_rm_user(const char *srv_name, const char *chat_name,
        const char *nick){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(nick);

    data->ui_interface = ui_rm_user_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->nick, nick, sizeof(data->nick));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_ren_user(const char *srv_name, const char *chat_name,
        const char *old_nick, const char *new_nick, UserType type){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(old_nick);
    g_return_if_fail(new_nick);

    data->ui_interface = ui_ren_user_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->nick, old_nick, sizeof(data->nick));
    strncpy(data->nick2, new_nick, sizeof(data->nick2));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_set_topic(const char *srv_name, const char *chat_name,
        const char *topic){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chat_name);
    g_return_if_fail(topic);

    data->ui_interface = ui_set_topic_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chat_name, chat_name, sizeof(data->chat_name));
    strncpy(data->msg, topic, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

/* ================================================================================ */
/* Note: the following functions are synchronous, should be called from main thread */
/* ================================================================================ */

/**
 * @brief Add a chatnel to main window
 *
 * @param srv_name Server's name, can't contains whitespace
 * @param chat_name Chatnel's name, can't contains whitespace
 *
 * @return 0 if successful, -1 if failed
 */
int ui_add_chat_sync(const char *srv_name, const char *chat_name,
        const char *nick, ChatType type){
    SrainChat *chat;
    chat = srain_window_add_chat(srain_win, srv_name, chat_name, type);

    if (chat){
        srain_chat_set_nick(chat, nick);
    }

    return chat != NULL ? 0 : -1;
}

/**
 * @brief Remove a chatnel from main window
 *
 * @param srv_name Server's name, can't contains whitespace
 * @param chat_name Chatnel's name, if chat_name == ""(empty string), remove
 *          all chatnel with the srv_name given as the argument
 *
 * @return 0 if successful, -1 if failed
 */
int ui_rm_chat_sync(const char *srv_name, const char *chat_name){
    GList *chats;
    SrainChat *chat;

    if (strcmp(chat_name, "") == 0){
        chats = srain_window_get_chats_by_srv_name(srain_win, srv_name);
        while (chats){
            srain_window_rm_chat(srain_win, chats->data);
            chats = g_list_next(chats);
        }

        g_list_free(chats);
    } else {
        chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
        if (chat == NULL){
            ERR_FR("No such chatnel: %s %s", srv_name, chat_name);
            return -1;
        }
        srain_window_rm_chat(srain_win, chat);
    }

    return 0;
}

/**
 * @brief Add a system message to sepcified chatnel
 *
 * @param srv_name
 * @param chat_name
 * @param msg
 * @param type SrainStackSidebar should be updated
 *             when type = SYS_MSG_ACTION or SYS_MSG_ERROR
 */
void ui_sys_msg_sync(const char *srv_name, const char *chat_name,
        const char *msg, SysMsgType type){
    int is_mentioned;
    int is_noitfy;
    const char *your_nick;
    SrainChat *chat;
    SrainMsgList *list;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    if (!chat){
        chat = srain_window_get_cur_chat(srain_win);

        /* Current SrainChat doesn't belong to `srv_name` */
        if (strcmp(srain_chat_get_srv_name(chat), srv_name) != 0){
            chat = srain_window_get_chat_by_name(srain_win, srv_name, META_SERVER);
        }
    }

    g_return_if_fail(chat);

    your_nick = srain_chat_get_nick(chat);

    /* is_mentioned = Message doesn't send by other user or server (It means a channel mesage)
     *                && you nick appeared in the message */
    is_mentioned = strlen(your_nick) != 0 && strstr(msg, your_nick) != NULL;

    /* `srv_name` == `id` means this message is sent by server.
     * is_notify = (Message sent by other user || your are mentioned in a channel)
     *             && window is not active   */
    is_noitfy = (strlen(your_nick) == 0 || is_mentioned)
        && !srain_window_is_active(srain_win);

    list = srain_chat_get_msg_list(chat);
    srain_msg_list_sys_msg_add(list, msg, type,
            type != SYS_MSG_NORMAL && is_mentioned);

    if (type != SYS_MSG_NORMAL){
        srain_window_stack_sidebar_update(srain_win, chat, NULL, msg);

        /* Desktop notification */
        if (is_noitfy){
            snotify_notify(type == SYS_MSG_ACTION ? _("ACTION") : _("ERROR"),
                    msg, "dialog-information");
            srain_window_tray_icon_stress(srain_win, 1);
        }
    }
}

/**
 * @brief Add a message to sepcified chatnel (sent by yourself)
 *
 * @param chat This chatnel must be existent
 * @param msg
 */
void ui_send_msg_sync(const char *srv_name, const char *chat_name, const char *msg){
    SrainChat *chat;
    SrainMsgList *list;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    g_return_if_fail(chat);
    list = srain_chat_get_msg_list(chat);

    srain_msg_list_send_msg_add(list, msg);
    srain_window_stack_sidebar_update(srain_win, chat, _("You"), msg);
}

/**
 * @brief Add a message to sepcified chatnel (sent by others),
 *      The nick will be added into the completion list of this chatnel,
 *      The sidebar should be updated
 *
 * @param chat_name If no such chatnel, fallback to META_SERVER
 * @param nick
 * @param id
 * @param msg
 */
void ui_recv_msg_sync(const char *srv_name, const char *chat_name,
        const char *nick, const char *id, const char *msg){
    int is_mentioned = 0;
    int is_noitfy = 0;
    const char *your_nick;
    SrainChat *chat;
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    if (!chat)
        chat = srain_window_get_chat_by_name(srain_win, srv_name, META_SERVER);
    g_return_if_fail(chat);

    your_nick = srain_chat_get_nick(chat);

    /* is_mentioned = Message doesn't send by other user or server (It means a channel mesage)
     *                && you nick appeared in the message */
    is_mentioned = strlen(your_nick) != 0 && strstr(msg, your_nick) != NULL;

    /* `srv_name` == `id` means this message is sent by server.
     * is_notify = Message doesn't send by server
     *             && (Message sent by other user || your are mentioned in a channel)
     *           && window is not active   */
    is_noitfy = strcmp(srv_name, id) != 0
        && (strlen(your_nick) == 0 || is_mentioned)
        && !srain_window_is_active(srain_win);

    list = srain_chat_get_msg_list(chat);
    srain_msg_list_recv_msg_add(list, nick, id, msg, is_mentioned);

    /* Do not sent notification when window is active */
    if (is_noitfy){
        snotify_notify(nick, msg, "dialog-information");
        srain_window_tray_icon_stress(srain_win, 1);
    }

    srain_window_stack_sidebar_update(srain_win, chat, nick, msg);
    if (strlen(id) != 0){
        comp = srain_chat_get_entry_completion(chat);
        if (!comp) return;
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_TMP);
    }

}

/**
 * @brief Add a nick into a specified chatnel's user list
 *
 * @param srv_name
 * @param chat_name
 * @param nick
 * @param type
 *
 * @return 0 if successful, -1 if failed
 */
int ui_add_user_sync(const char *srv_name, const char *chat_name,
        const char *nick, UserType type){
    int res;
    SrainChat *chat;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    if (chat == NULL){
        ERR_FR("No such chatnel: %s %s", srv_name, chat_name);
        return -1;
    }

    list = srain_chat_get_user_list(chat);

    if ((res = srain_user_list_add(list, nick, type)) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return res;
}

/**
 * @brief Remove a nick from a specified chatnel's user list
 *
 * @param srv_name
 * @param chat_name
 * @param nick
 *
 * @return 0 if successful, -1 if failed
 */
int ui_rm_user_sync(const char *srv_name, const char *chat_name,
        const char *nick){
    int res;
    SrainChat *chat;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    if (chat == NULL){
        ERR_FR("No such chatnel: %s %s", srv_name, chat_name);
        return -1;
    }

    list = srain_chat_get_user_list(chat);

    if ((res = srain_user_list_rm(list, nick)) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return res;
}

/**
 * @brief Rename a item in all chatnels' user list
 *
 * @param srv_name
 * @param chat_name
 * @param old_nick
 * @param new_nick
 * @param type
 * @param msg When nick was renamed in a chatnel, send `reason`
 *          to this chatnel using `ui_sys_msg()`
 */
void ui_ren_user_sync(const char *srv_name, const char *chat_name,
        const char *old_nick, const char *new_nick, UserType type){
    SrainChat *chat;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    g_return_if_fail(chat);

    /* Your nick changed */
    if (strcmp(old_nick, srain_chat_get_nick(chat)) == 0){
        srain_chat_set_nick(chat, new_nick);
    }

    chat_name = srain_chat_get_chat_name(chat);
    list = srain_chat_get_user_list(chat);

    if (srain_user_list_rename(list, old_nick, new_nick, type) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }
}

/**
 * @brief Set topic
 *
 * @param srv_name
 * @param chat_name
 * @param topic
 */
void ui_set_topic_sync(const char *srv_name, const char *chat_name, const char *topic){
    SrainChat *chat;

    chat = srain_window_get_chat_by_name(srain_win, srv_name, chat_name);
    g_return_if_fail(chat);
    srain_chat_set_topic(chat, topic);
}
