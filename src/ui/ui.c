/**
 * @file ui.c
 * @brief UI module interface
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-29
 */

#include <gtk/gtk.h>
#include <string.h>

#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"

#include "i18n.h"
#include "log.h"

/**
 * @brief Add a channel to main window
 *
 * @param server_name Server's name, can't contains whitespace
 * @param chan_name Channel's name, can't contains whitespace
 *
 * @return 0 if successful, -1 if failed
 */
int
ui_add_chan(const char *server_name, const char *chan_name){
    SrainChan *chan = NULL;

    chan = srain_window_add_chan(srain_win, server_name, chan_name);

    return chan ? 0 : -1;
}

/**
 * @brief Remove a channel from main window
 *
 * @param server_name Server's name, can't contains whitespace
 * @param chan_name Channel's name, can't contains whitespace
 *
 * @return 0 if successful, -1 if failed
 */
int
ui_rm_chan(const char *server_name, const char *chan_name){
    SrainChan *chan;
    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);

    if (!chan) return -1;
    srain_window_rm_chan(srain_win, chan);

    return 0;
}

/**
 * @brief Add a system message to sepcified channel
 *
 * @param server_name
 * @param chan_name If no such channel, fallback to current one
 * @param msg
 * @param type If it is SYS_MSG_ACTION, sidebar should be updated
 */
void
ui_sys_msg(const char *server_name, const char *chan_name,
        const char *msg, SysMsgType type){
    SrainChan *chan;
    SrainMsgList *list;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    if (chan == NULL) chan = srain_window_get_cur_chan(srain_win);

    if (!list) return;
    srain_msg_list_sys_msg_add(list, msg, type);

    if (type == SYS_MSG_ACTION){
        srain_window_stack_sidebar_update(srain_win, chan, _("ACTION"), msg);
    }
}

/**
 * @brief Add a message to sepcified channel (sent by yourself)
 *
 * @param chan This channel must be existent
 * @param msg
 */
void
ui_send_msg(const char *server_name, const char *chan_name, const char *msg){
    SrainChan *chan;
    SrainMsgList *list;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    list = srain_chan_get_msg_list(chan);
    if (!list) return;

    srain_msg_list_send_msg_add(list, msg);
    srain_window_stack_sidebar_update(srain_win, chan, _("You"), msg);
}

/**
 * @brief Add a message to sepcified channel (sent by others),
 *      The nick will be added into the completion list of this channel,
 *      The sidebar should be updated
 *
 * @param chan
 * @param nick
 * @param id
 * @param msg
 */
void
ui_recv_msg(const char *server_name, const char *chan_name,
        const char *nick, const char *id, const char *msg){
    SrainChan *chan;
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    list = srain_chan_get_msg_list(chan);

    srain_msg_list_recv_msg_add(list, nick, id, msg);
    srain_window_stack_sidebar_update(srain_win, chan, nick, msg);

    if (strlen(id) != 0){
        comp = srain_chan_get_entry_completion(chan);
        if (!comp) return;
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_TMP);
    }
}

/**
 * @brief Add a nick into a specified channel's user list
 *
 * @param server_name
 * @param chan_name
 * @param nick
 * @param type
 *
 * @return 0 if successful, -1 if failed
 */
int
ui_user_list_add(const char *server_name, const char *chan_name,
        const char *nick, IRCUserType type){
    int res;
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    list = srain_chan_get_user_list(chan);

    if ((res = srain_user_list_add(list, nick, type)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return res;
}

/**
 * @brief Remove a nick from a specified channel's user list
 *
 * @param server_name
 * @param chan_name
 * @param nick
 * @param reason
 *
 * @return 0 if successful, -1 if failed
 */
int
ui_user_list_rm(const char *server_name, const char *chan_name,
        const char *nick, const char *reason){
    int res;
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    list = srain_chan_get_user_list(chan);

    if ((res = srain_user_list_rm(list, nick)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return res;
}

/**
 * @brief Rename a item in a specified channel's user list
 *
 * @param server_name
 * @param chan_name
 * @param old_nick
 * @param new_nick
 *
 * @return 0 if successful, -1 if failed
 */
int
ui_user_list_rename(const char *server_name, const char *chan_name,
        const char *old_nick, const char *new_nick){
    int res;
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    list = srain_chan_get_user_list(chan);

    if ((res = srain_user_list_rename(list, old_nick, new_nick)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }

    return res;
}

/**
 * @brief Set topic
 *
 * @param server_name
 * @param chan_name
 * @param topic
 */
void
ui_set_topic(const char *server_name, const char *chan_name, const char *topic){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(srain_win, server_name, chan_name);
    if (chan) srain_chan_set_topic(chan, topic);
}
