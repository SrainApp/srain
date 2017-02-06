/* @file ui.c
 * @brief UI module interfaces
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-06-29
 */

// #define __DBG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "ui.h"
#include "ui_common.h"
#include "srain_app.h"
#include "srain_window.h"
#include "theme.h"
#include "snotify.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"

void ui_init(int argc, char **argv){
    theme_init();
    snotify_init();

    g_application_run(G_APPLICATION(srain_app_new()), argc, argv);

    snotify_finalize();
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
void* ui_add_chat(Server *srv, const char *name, ChatType type){
    SrainChat *chat;
    chat = srain_window_add_chat(srain_win, srv->name, name, type);

    if (chat){
        srain_chat_set_nick(chat, srv->user.nick);
    }

    return chat;
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
void ui_rm_chat(void *chat){
    g_return_if_fail(SRAIN_IS_CHAT(chat));
    srain_window_rm_chat(srain_win, chat);
}

/**
 * @brief Add a system message to sepcified chatnel
 *
 * @param srv_name
 * @param chat_name
 * @param msg
 * @param type SrainStackSidebar should be updated
 *             when type = SYS_MSG_ACTION or SYS_MSG_ERROR
 * @param flag
 */
void ui_sys_msg(void *chat, const char *msg, SysMsgType type, SrainMsgFlag flag){
    SrainMsgList *list;

    if (!chat){
        if (type == SYS_MSG_ERROR){
            char buf[512];
            snprintf(buf, sizeof(buf), _("<b>Session:</b> %s\n<b>Error message:</b> %s"),
                    "TODO", msg);
            show_msg_dialog(_("ERROR"), buf);
        }
        return;
    }

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    list = srain_chat_get_msg_list(chat);
    srain_msg_list_sys_msg_add(list, msg, type, flag);

    if (type != SYS_MSG_NORMAL){
        srain_window_stack_sidebar_update(srain_win, chat, NULL, msg);
    }
}

/**
 * @brief Add a message to sepcified chatnel (sent by yourself)
 *
 * @param chat This chatnel must be existent
 * @param msg
 * @param flag
 */
void ui_send_msg(void *chat, const char *msg, SrainMsgFlag flag){
    SrainMsgList *list;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    list = srain_chat_get_msg_list(chat);

    srain_msg_list_send_msg_add(list, msg, flag);
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
 * @param flag
 */
void ui_recv_msg(void *chat, const char *nick, const char *id, const char *msg,
        SrainMsgFlag flag){
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    g_return_if_fail(SRAIN_IS_CHAT(chat));
    
    list = srain_chat_get_msg_list(chat);
    srain_msg_list_recv_msg_add(list, nick, id, msg, flag);

    // TODO: move in srain_msg_list?
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
int ui_add_user(void *chat, const char *nick, UserType type){
    int res;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SRAIN_IS_CHAT(chat), SRN_ERR);

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
int ui_rm_user(void *chat, const char *nick){
    int res;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SRAIN_IS_CHAT(chat), SRN_ERR);
    
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
void ui_ren_user(void *chat, const char *old_nick, const char *new_nick, UserType type){
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    /* Your nick changed */
    if (strcmp(old_nick, srain_chat_get_nick(chat)) == 0){
        srain_chat_set_nick(chat, new_nick);
    }

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
void ui_set_topic(void *chat, const char *topic){
    g_return_if_fail(SRAIN_IS_CHAT(chat));

    srain_chat_set_topic(chat, topic);
}
