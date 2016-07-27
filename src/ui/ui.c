/**
 * @file ui.c
 * @brief UI module interface
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-06-29
 */

#include <gtk/gtk.h>
#include <string.h>

#include "ui.h"
#include "srain_app.h"
#include "srain_chan.h"
#include "srain_window.h"

#include "i18n.h"
#include "log.h"

/**
 * @brief Add a channel to main window
 *
 * @param srv_name Server's name, can't contains whitespace
 * @param chan_name Channel's name, can't contains whitespace
 */
void ui_add_chan(const char *srv_name, const char *chan_name){
    srain_window_add_chan(srain_win, srv_name, chan_name);
}

/**
 * @brief Remove a channel from main window
 *
 * @param srv_name Server's name, can't contains whitespace
 * @param chan_name Channel's name, can't contains whitespace
 */
void ui_rm_chan(const char *srv_name, const char *chan_name){
    SrainChan *chan;
    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);

    g_return_if_fail(chan);
    srain_window_rm_chan(srain_win, chan);
}

/**
 * @brief Add a system message to sepcified channel
 *
 * @param srv_name
 * @param chan_name If no such channel, fallback to current one
 * @param msg
 * @param type If it is SYS_MSG_ACTION, sidebar should be updated
 */
void
ui_sys_msg(const char *srv_name, const char *chan_name,
        const char *msg, SysMsgType type){
    SrainChan *chan;
    SrainMsgList *list;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    if (chan == NULL) chan = srain_window_get_cur_chan(srain_win);
    g_return_if_fail(chan);

    list = srain_chan_get_msg_list(chan);
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
ui_send_msg(const char *srv_name, const char *chan_name, const char *msg){
    SrainChan *chan;
    SrainMsgList *list;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    g_return_if_fail(chan);
    list = srain_chan_get_msg_list(chan);

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
ui_recv_msg(const char *srv_name, const char *chan_name,
        const char *nick, const char *id, const char *msg){
    SrainChan *chan;
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    g_return_if_fail(chan);
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
 * @param srv_name
 * @param chan_name
 * @param nick
 * @param type
 */
void
ui_user_list_add(const char *srv_name, const char *chan_name,
        const char *nick, UserType type){
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    g_return_if_fail(chan);
    list = srain_chan_get_user_list(chan);

    if (srain_user_list_add(list, nick, type) == 0){
        comp = srain_chan_get_entry_completion(chan);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };
}

/**
 * @brief Remove a nick from a specified channel's user list
 *
 * @param srv_name
 * @param chan_name
 * @param nick
 */
void ui_user_list_rm(const char *srv_name, const char *chan_name,
        const char *nick){
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    g_return_if_fail(chan);
    list = srain_chan_get_user_list(chan);

    if (srain_user_list_rm(list, nick) == 0){
        comp = srain_chan_get_entry_completion(chan);
        srain_entry_completion_rm_keyword(comp, nick);
    }
}

/**
 * @brief Remove a nick from all channels' user list
 *
 * @param srv_name
 * @param nick
 * @param reason When nick was removed form a channel, send `reason`
 *          to this channel using `ui_sys_msg()`
 */
void ui_user_list_rm_all(const char *srv_name, const char *nick,
        const char *reason){
    const char *chan_name;
    GList *chans;

    chans = srain_window_get_chans_by_srv_name(srain_win, srv_name);
    g_return_if_fail(chans);
    while (chans){
        chan_name = srain_chan_get_chan_name(SRAIN_CHAN(chans->data));
        ui_user_list_rm(srv_name, chan_name, nick);
        ui_sys_msg(srv_name, chan_name, reason, SYS_MSG_NORMAL);

        chans = g_list_next(chans);
    }

    g_list_free(chans);
}

/**
 * @brief Rename a item in all channels' user list
 *
 * @param srv_name
 * @param old_nick
 * @param new_nick
 * @param type
 * @param msg When nick was renamed in a channel, send `reason`
 *          to this channel using `ui_sys_msg()`
 */
void ui_user_list_rename(const char *srv_name, const char *old_nick,
        const char *new_nick, UserType type, const char *msg){
    const char *chan_name;
    GList *chans;
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chans = srain_window_get_chans_by_srv_name(srain_win, srv_name);
    g_return_if_fail(chans);

    while (chans){
        chan = SRAIN_CHAN(chans->data);
        list = srain_chan_get_user_list(chan);

        if (srain_user_list_rename(list, old_nick, new_nick, type) == 0){
            comp = srain_chan_get_entry_completion(chan);
            srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
            srain_entry_completion_rm_keyword(comp, new_nick);
        }

        chans = g_list_next(chans);
    }

    g_list_free(chans);
}

/**
 * @brief Set topic
 *
 * @param srv_name
 * @param chan_name
 * @param topic
 */
void
ui_set_topic(const char *srv_name, const char *chan_name, const char *topic){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    g_return_if_fail(chan);
    srain_chan_set_topic(chan, topic);
}
