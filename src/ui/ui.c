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
#include "srain_chan.h"
#include "srain_window.h"
#include "theme.h"

#include "srv_session.h"

#include "i18n.h"
#include "log.h"
#include "meta.h"

typedef struct {
    void *ui_interface;
    char srv_name[HOST_LEN];
    char chan_name[CHAN_LEN];
    char nick[NICK_LEN];
    char nick2[NICK_LEN];
    char msg[MSG_LEN];
    int type;
} CommonUIData;

void ui_init(int argc, char **argv){
    ui_hdr_init();
    theme_init();

    g_application_run(G_APPLICATION(srain_app_new()), argc, argv);
}

void ui_idle_destroy_data(void *data){
    DBG_FR("CommonUIData %p freed", data);
    g_free(data);
}

int ui_idle(CommonUIData *data){
    DBG_FR("Idle call, data: %p", data);
    DBG_FR("func: %p", data->ui_interface);

    if (data->ui_interface == ui_add_chan_sync){
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        ui_add_chan_sync(srv_name, chan_name);
    }
    else if (data->ui_interface == ui_rm_chan_sync){
        DBG_FR("ui_rm_chan_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        ui_rm_chan_sync(srv_name, chan_name);
    }
    else if (data->ui_interface == ui_sys_msg_sync){
        DBG_FR("ui_sys_msg_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        const char *msg = data->msg;
        SysMsgType type = data->type;
        ui_sys_msg_sync(srv_name, chan_name, msg, type);
    }
    else if (data->ui_interface == ui_send_msg_sync){
        DBG_FR("ui_send_msg_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        const char *msg = data->msg;
        ui_send_msg_sync(srv_name, chan_name, msg);
    }
    else if (data->ui_interface == ui_recv_msg_sync){
        DBG_FR("ui_recv_msg_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        const char *nick = data->nick;
        const char *id = data->nick2;
        const char *msg = data->msg;
        ui_recv_msg_sync(srv_name, chan_name, nick, id, msg);
    }
    else if (data->ui_interface == ui_user_list_add_sync){
        DBG_FR("ui_user_list_add_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        const char *nick = data->nick;
        UserType type = data->type;
        ui_user_list_add_sync(srv_name, chan_name, nick, type);
    }
    else if (data->ui_interface == ui_user_list_rm_sync){
        DBG_FR("ui_user_list_rm_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        const char *nick = data->nick;
        ui_user_list_rm_sync(srv_name, chan_name, nick);
    }
    else if (data->ui_interface == ui_user_list_rm_all_sync){
        DBG_FR("ui_user_list_rm_all_sync");
        const char *srv_name = data->srv_name;
        const char *nick = data->nick;
        const char *reason = data->msg;
        ui_user_list_rm_all_sync(srv_name, nick, reason);
    }
    else if (data->ui_interface == ui_user_list_rename_sync){
        DBG_FR("ui_user_list_rename_sync");
        const char *srv_name = data->srv_name;
        const char *nick = data->nick;
        const char *new_nick = data->nick2;
        const char *reason = data->msg;
        UserType type = data->type;
        ui_user_list_rename_sync(srv_name, nick, new_nick, type, reason);
    }
    else if (data->ui_interface == ui_set_topic_sync){
        DBG_FR("ui_set_topic_sync");
        const char *srv_name = data->srv_name;
        const char *chan_name = data->chan_name;
        const char *topic = data->msg;
        ui_set_topic_sync(srv_name, chan_name, topic);
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

void ui_add_chan(const char *srv_name, const char *chan_name){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);

    data->ui_interface = ui_add_chan_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_rm_chan(const char *srv_name, const char *chan_name){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);

    data->ui_interface = ui_rm_chan_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_sys_msg(const char *srv_name, const char *chan_name,
        const char *msg, SysMsgType type){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);
    g_return_if_fail(msg);

    data->ui_interface = ui_sys_msg_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));
    strncpy(data->msg, msg, sizeof(data->msg));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_send_msg(const char *srv_name, const char *chan_name,
        const char *msg){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);
    g_return_if_fail(msg);

    data->ui_interface = ui_send_msg_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));
    strncpy(data->msg, msg, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_recv_msg(const char *srv_name, const char *chan_name,
        const char *nick, const char *id, const char *msg){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);
    g_return_if_fail(nick);
    g_return_if_fail(msg);
    g_return_if_fail(id);

    data->ui_interface = ui_recv_msg_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));
    strncpy(data->nick, nick, sizeof(data->nick));
    strncpy(data->nick2, id, sizeof(data->msg));
    strncpy(data->msg, msg, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_user_list_add(const char *srv_name, const char *chan_name,
 const char *nick, UserType type){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);
    g_return_if_fail(nick);

    data->ui_interface = ui_user_list_add_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));
    strncpy(data->nick, nick, sizeof(data->nick));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_user_list_rm(const char *srv_name, const char *chan_name,
        const char *nick){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);
    g_return_if_fail(nick);

    data->ui_interface = ui_user_list_rm_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));
    strncpy(data->nick, nick, sizeof(data->nick));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_user_list_rm_all(const char *srv_name, const char *nick,
        const char *reason){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(nick);
    g_return_if_fail(reason);

    data->ui_interface = ui_user_list_rm_all_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->nick, nick, sizeof(data->nick));
    strncpy(data->msg, reason, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_user_list_rename(const char *srv_name, const char *old_nick,
        const char *new_nick, UserType type, const char *msg){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(old_nick);
    g_return_if_fail(new_nick);
    g_return_if_fail(msg);

    data->ui_interface = ui_user_list_rename_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->nick, old_nick, sizeof(data->nick));
    strncpy(data->nick2, new_nick, sizeof(data->nick2));
    strncpy(data->msg, msg, sizeof(data->msg));
    data->type = type;

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

void ui_set_topic(const char *srv_name, const char *chan_name,
        const char *topic){
    CommonUIData *data = g_malloc0(sizeof(CommonUIData));

    g_return_if_fail(srv_name);
    g_return_if_fail(chan_name);
    g_return_if_fail(topic);

    data->ui_interface = ui_set_topic_sync;
    strncpy(data->srv_name, srv_name, sizeof(data->srv_name));
    strncpy(data->chan_name, chan_name, sizeof(data->chan_name));
    strncpy(data->msg, topic, sizeof(data->msg));

    gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)ui_idle, data, ui_idle_destroy_data);
}

/* ================================================================================ */
/* Note: the following functions are synchronous, should be called from main thread */
/* ================================================================================ */

/**
 * @brief Add a channel to main window
 *
 * @param srv_name Server's name, can't contains whitespace
 * @param chan_name Channel's name, can't contains whitespace
 *
 * @return 0 if successful, -1 if failed
 */
int ui_add_chan_sync(const char *srv_name, const char *chan_name){
    return srain_window_add_chan(srain_win, srv_name, chan_name)
        ? 0 : -1;
}

/**
 * @brief Remove a channel from main window
 *
 * @param srv_name Server's name, can't contains whitespace
 * @param chan_name Channel's name, if chan_name == ""(empty string), remove
 *          all channel with the srv_name given as the argument
 *
 * @return 0 if successful, -1 if failed
 */
int ui_rm_chan_sync(const char *srv_name, const char *chan_name){
    GList *chans;
    SrainChan *chan;

    if (strcmp(chan_name, "") == 0){
        chans = srain_window_get_chans_by_srv_name(srain_win, srv_name);
        while (chans){
            srain_window_rm_chan(srain_win, chans->data);
            chans = g_list_next(chans);
        }

        g_list_free(chans);
    } else {
        chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
        if (chan == NULL){
            ERR_FR("No such channel: %s %s", srv_name, chan_name);
            return -1;
        }
        srain_window_rm_chan(srain_win, chan);
    }

    return 0;
}

/**
 * @brief Add a system message to sepcified channel
 *
 * @param srv_name
 * @param chan_name If no such channel, fallback to current one
 * @param msg
 * @param type If it is SYS_MSG_ACTION, sidebar should be updated
 */
void ui_sys_msg_sync(const char *srv_name, const char *chan_name,
        const char *msg, SysMsgType type){
    SrainChan *chan;
    SrainMsgList *list;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    if (chan == NULL) chan = srain_window_get_cur_chan(srain_win);
    g_return_if_fail(chan);

    list = srain_chan_get_msg_list(chan);
    srain_msg_list_sys_msg_add(list, msg, type);

    if (type == SYS_MSG_ACTION || type== SYS_MSG_ERROR){
        srain_window_stack_sidebar_update(srain_win, chan, NULL, msg);
    }
}

/**
 * @brief Add a message to sepcified channel (sent by yourself)
 *
 * @param chan This channel must be existent
 * @param msg
 */
void ui_send_msg_sync(const char *srv_name, const char *chan_name, const char *msg){
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
 * @param chan_name If no such channel, fallback to META_SERVER
 * @param nick
 * @param id
 * @param msg
 */
void ui_recv_msg_sync(const char *srv_name, const char *chan_name,
        const char *nick, const char *id, const char *msg){
    SrainChan *chan;
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    if (!chan)
        chan = srain_window_get_chan_by_name(srain_win, srv_name, META_SERVER);
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
 *
 * @return 0 if successful, -1 if failed
 */
int ui_user_list_add_sync(const char *srv_name, const char *chan_name,
        const char *nick, UserType type){
    int res;
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    if (chan == NULL){
        ERR_FR("No such channel: %s %s", srv_name, chan_name);
        return -1;
    }

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
 * @param srv_name
 * @param chan_name
 * @param nick
 *
 * @return 0 if successful, -1 if failed
 */
int ui_user_list_rm_sync(const char *srv_name, const char *chan_name,
        const char *nick){
    int res;
    SrainChan *chan;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    if (chan == NULL){
        ERR_FR("No such channel: %s %s", srv_name, chan_name);
        return -1;
    }

    list = srain_chan_get_user_list(chan);

    if ((res = srain_user_list_rm(list, nick)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return res;
}

/**
 * @brief Remove a nick from all channels' user list
 *
 * @param srv_name
 * @param nick
 * @param reason When nick was removed form a channel, send `reason`
 *          to this channel using `ui_sys_msg()`
 */
void ui_user_list_rm_all_sync(const char *srv_name, const char *nick,
        const char *reason){
    const char *chan_name;
    GList *chans;

    DBG_FR("%s %s %s", srv_name, nick, reason);

    chans = srain_window_get_chans_by_srv_name(srain_win, srv_name);
    while (chans){
        chan_name = srain_chan_get_chan_name(SRAIN_CHAN(chans->data));
        if (ui_user_list_rm_sync(srv_name, chan_name, nick) == 0){
            ui_sys_msg(srv_name, chan_name, reason, SYS_MSG_NORMAL);
        }

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
void ui_user_list_rename_sync(const char *srv_name, const char *old_nick,
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
        chan_name = srain_chan_get_chan_name(chan);
        list = srain_chan_get_user_list(chan);

        if (srain_user_list_rename(list, old_nick, new_nick, type) == 0){
            comp = srain_chan_get_entry_completion(chan);
            srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
            srain_entry_completion_rm_keyword(comp, new_nick);

            ui_sys_msg(srv_name, chan_name, msg, SYS_MSG_NORMAL);
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
void ui_set_topic_sync(const char *srv_name, const char *chan_name, const char *topic){
    SrainChan *chan;

    chan = srain_window_get_chan_by_name(srain_win, srv_name, chan_name);
    g_return_if_fail(chan);
    srain_chan_set_topic(chan, topic);
}
