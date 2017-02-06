/**
 * @file irc.c
 * @brief IRC module
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-02
 *
 */

#define __LOG_ON
#define __DBG_ON

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "socket.h"
#include "irc_magic.h"
#include "irc_cmd.h"
#include "irc_parse.h"
#include "irc_event.h"

#include "srain.h"
#include "log.h"

static void th_irc_connect(Server *srv);
static void th_irc_proc(Server *srv);
static void th_irc_recv(Server *srv);
static int irc_recv(Server *srv);

int irc_init(){
    irc_event_init();
}

void irc_connect(Server *srv){
    srv->stat = SERVER_CONNECTING;
    g_thread_new(NULL, (GThreadFunc)th_irc_connect, srv);
}

void irc_disconnect(Server *srv){
    srv->stat = SERVER_DISCONNECTED;
    close(srv->fd);
    srv->fd = -1;
}

/* This function runs on a separate thread, blocking read a line
 * (terminated with "\r\n") from socket and then pass it to irc_recv(), which
 * runs on GTK main thread. */

static void th_irc_connect(Server *srv){
    struct timeval tv = { 1, 0 }; // 1 second 

    srv->fd = sck_get_socket(srv->host, srv->port);

    if (srv->fd < 0){
        ERR_FR("Failed to connect to %s:%d", srv->host, srv->port);
        srv->stat = SERVER_DISCONNECTED;
        return;
    }

    srv->stat = SERVER_CONNECTED;

    th_irc_proc(srv);
}

static void th_irc_proc(Server *srv){
    for (;;){
        th_irc_recv(srv);
        if (srv->stat == SERVER_DISCONNECTED) {
            WARN_FR("Server %s Thread exit", srv->name);
            return;
        }
    }
}

static void th_irc_recv(Server *srv){
    g_mutex_lock(&srv->mutex);
    int ret = sck_readline(srv->fd, srv->buf, sizeof(srv->buf));

    if (ret != SRN_ERR){
        gdk_threads_add_idle_full(G_PRIORITY_HIGH_IDLE,
                (GSourceFunc)irc_recv, srv, NULL);
    } else {
        ERR_FR("Socket error, connection close");
        g_mutex_unlock(&srv->mutex);
        srv->stat = SERVER_DISCONNECTED;
    }
}

static int irc_recv(Server *srv){
    IRCMsg imsg;

    int res = irc_parse(srv->buf, &imsg);

    switch (res){
        case IRCMSG_PING:
            irc_cmd_pong(srv->fd, srv->buf);
            // TODO: reconnect when ping time out.
            break;
        default:
            irc_event_hdr(srv, &imsg);
            break;
    }

    g_mutex_unlock(&srv->mutex);
    return FALSE;
}
