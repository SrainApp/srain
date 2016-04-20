/**
 * @file ui_intf.c
 * @brief UI modeule interface
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-21
 */
#define __LOG_ON

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "log.h"

extern SrainApp *srain_app;
extern SrainWindow *srain_win;

/**
 * @brief ui_intf_server_cmd interface to execute a command
 *
 * @param source where this cmd comes from,
 *      if NULL, command echo may appear in the current SrainChan
 * @param cmd the command you want to execute
 *
 * @return 0 if successed, -1 if failed
 */
int ui_intf_server_cmd(SrainChan *chan, const char *cmd){
    int res;
    void *srv;
    char *cmd2;
    const char *chan_name;

    cmd2 = strdup(cmd);

    if (chan){
        srv = g_object_get_data(G_OBJECT(chan), "server");
        chan_name = gtk_widget_get_name(GTK_WIDGET(chan));
        res = srain_app->server_cmd(srv, chan_name, cmd2);
    } else {
        res = srain_app->server_cmd(NULL, NULL, cmd2);
    }

    free(cmd2);
    return res;
}

/**
 * @brief ui_intf_server_join join a channel of current server
 *
 * @param chan_name
 */
void ui_intf_server_join(const char *chan_name){
    void *srv;
    SrainChan *chan;

    chan = srain_window_get_cur_chan(srain_win);
    srv = g_object_get_data(G_OBJECT(chan), "server");

    srain_app->server_join(srv, chan_name);
}

void ui_intf_server_part(SrainChan *chan){
    void *srv;
    const char *chan_name;

    srv = g_object_get_data(G_OBJECT(chan), "server");
    chan_name = gtk_widget_get_name(GTK_WIDGET(chan));

    srain_app->server_part(srv, chan_name);
}

void ui_intf_server_send(SrainChan *chan, const char *msg){
    void *srv;
    const char *chan_name;

    if (chan == NULL){
        ERR_FR("chan: NULL, msg: %s", msg);
        return;
    }

    srv = g_object_get_data(G_OBJECT(chan), "server");
    chan_name = gtk_widget_get_name(GTK_WIDGET(chan));

    srain_app->server_send(srv, chan_name, msg);
}
