/**
 * @file srain_app.c
 * @brief Srain's application class implement
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>

#include "theme.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_user_list.h"
#include "srain_msg_list.h"
#include "srain_entry_completion.h"

#include "server.h"
#include "server_cmd.h"

#include "meta.h"
#include "rc.h"
#include "log.h"
#include "i18n.h"

G_DEFINE_TYPE(SrainApp, srain_app, GTK_TYPE_APPLICATION);

/* Only one SrainApp instance in one application */
SrainApp *srain_app = NULL;
SrainWindow *srain_win = NULL;

static void srain_app_init(SrainApp *self){
    if (srain_app) return;

    self->server_join = (ServerJoinFunc)server_join;
    self->server_part = (ServerPartFunc)server_part;
    self->server_send = (ServerSendFunc)server_send;
    self->server_cmd = (ServerCmdFunc)server_cmd;

    srain_app = self;

    return;
}

static void srain_app_activate(GtkApplication *app){
    if (srain_win){
        gtk_window_present(GTK_WINDOW(srain_win));
    } else {
        theme_init();
        srain_win = srain_window_new(SRAIN_APP(app));
        gtk_window_present(GTK_WINDOW(srain_win));
        rc_read();
    }
}

static void srain_app_class_init(SrainAppClass *class){
    G_APPLICATION_CLASS(class)->activate = (void *)(GApplication *)srain_app_activate;
}

SrainApp* srain_app_new(void){
    return g_object_new(SRAIN_TYPE_APP, "application-id", "org.gtk.srain", NULL);
}

SrainChan* srain_app_add_chan(void *server, const char *server_name, const char *chan_name){
    SrainChan *chan;

    chan = srain_window_add_chan(srain_win, server_name, chan_name);
    g_object_set_data(G_OBJECT(chan), "server", server);

    return chan;
}

void srain_app_rm_chan(SrainChan *chan){
    return srain_window_rm_chan(srain_win, chan);
}

/**
 * @brief srain_app_sys_msg UI interface used to send system message
 *
 * @param chan if NULL, fallback to current chan,
 *      if current chan is NULL too, giveup this message
 * @param msg
 * @param type if SYS_MSG_ACTION, this message should be display on sidebar
 */
void srain_app_sys_msg(SrainChan *chan, const char *msg, SysMsgType type){
    SrainMsgList *list;

    if (chan == NULL) chan = srain_window_get_cur_chan(srain_win);
    if (chan == NULL) {
        ERR_FR("chan: (null), msg: '%s', type: %d, current chan is null",
                msg, type);
    }

    list = srain_chan_get_msg_list(chan);
    if (!list) return;

    srain_msg_list_sys_msg_add(list, msg, type);

    if (type == SYS_MSG_ACTION){
        srain_window_stack_sidebar_update(srain_win, chan, _("ACTION"), msg);
    }
}

/**
 * @brief srain_app_send_msg UI interface used to send sent message
 *
 * @param chan
 * @param msg
 */
void srain_app_send_msg(SrainChan *chan, const char *msg){
    SrainMsgList *list;

    list = srain_chan_get_msg_list(chan);
    if (!list) return;

    srain_msg_list_send_msg_add(list, msg);
    srain_window_stack_sidebar_update(srain_win, chan, _("You"), msg);
}

/**
 * @brief srain_app_recv_msg UI interface uesd to send recvived message
 *
 * @param chan
 * @param nick
 * @param id
 * @param msg
 */
void srain_app_recv_msg(SrainChan *chan, const char *nick, const char *id, const char *msg){
    SrainMsgList *list;

    list = srain_chan_get_msg_list(chan);
    if (!list) return;

    srain_msg_list_recv_msg_add(list, nick, id, msg);
    srain_window_stack_sidebar_update(srain_win, chan, nick, msg);
}

int srain_app_user_list_add(SrainChan *chan, const char *nick, IRCUserType type){
    int res;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    list = srain_chan_get_user_list(chan);
    if (!list) return -1;

    if ((res = srain_user_list_add(list, nick, type)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        if (!comp) return -1;
        srain_entry_completion_add_keyword(comp, nick);
    };

    return res;
}

int srain_app_user_list_rm(SrainChan *chan, const char *nick, const char *reason){
    int res;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    list = srain_chan_get_user_list(chan);
    if (!list) return -1;

    if ((res = srain_user_list_rm(list, nick)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        if (!comp) return -1;
        srain_entry_completion_add_keyword(comp, nick);
    }

    return res;
}

int srain_app_user_list_rename(SrainChan *chan, const char *old_nick, const char *new_nick){
    int res;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    list = srain_chan_get_user_list(chan);
    if (!list) return -1;

    if ((res = srain_user_list_rename(list, old_nick, new_nick)) == 0){
        comp = srain_chan_get_entry_completion(chan);
        if (!comp) return -1;
        srain_entry_completion_add_keyword(comp, old_nick);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }

    return res;
}

void srain_app_set_topic(SrainChan *chan, const char *topic){
    return srain_chan_set_topic(chan, topic);
}
