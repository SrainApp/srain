/**
 * @file sirc.c
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

#include "socket.h"
#include "irc_magic.h"
#include "irc_cmd.h"
#include "irc_parse.h"
#include "irc_event.h"

#include "srain.h"
#include "log.h"

typedef struct {
    SircSession *sirc;
    char *host;
    int port;
} ThreadData;

/* NOTE: The "th_" prefix means that this function invoked in a seprate thread.
 */
static void th_sirc_connect(ThreadData *td);
static void th_sirc_proc(SircSession *sirc);
static void th_sirc_recv(SircSession *sirc);

/* NOTE: The "idle_" prefix means that this function invoked as a idle function
 * in GLib main thread(main loop).
 */
static int idle_sirc_on_connect(SircSession *sirc);
static int idle_sirc_on_disconnect(SircSession *sirc);
static int idle_sirc_recv(SircSession *sirc);

int irc_init(){
    return SRN_OK;
}

SircSession* sirc_new(void *ctx){
    SircSession *sirc = g_malloc0(sizeof(SircSession));

    sirc->fd = -1;
    sirc->ctx = ctx;

    g_mutex_init(&sirc->mutex);

    return sirc;
}

SircSession* sirc_free(SircSession *sirc){
    if (sirc->fd != -1){
        sirc_disconnect(sirc);
    }

    g_free(sirc);
}

void sirc_connect(SircSession *sirc, const char *host, int port){
    ThreadData *td = g_malloc0(sizeof(ThreadData)); // Free by th_irc_connect

    td->sirc = sirc;
    td->host = g_strdup(host);
    td->port = port;

    g_thread_new(NULL, (GThreadFunc)th_sirc_connect, td);
}

void sirc_disconnect(SircSession *sirc){
    close(sirc->fd);
    sirc->fd = -1;
}

/******************************************************************************
 * Functions runs in separate thread
 *****************************************************************************/

static void th_sirc_connect(ThreadData *td){
    SircSession *sirc = td->sirc;

    sirc->fd = sck_get_socket(td->host, td->port);

    if (sirc->fd < 0){
        ERR_FR("Failed to connect to %s:%d", td->host, td->port);
        g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                (GSourceFunc)idle_sirc_on_disconnect, sirc, NULL);
        return;
    }

    g_free(td->host);
    g_free(td);

    g_idle_add_full(G_PRIORITY_HIGH_IDLE,
            (GSourceFunc)idle_sirc_on_connect, sirc, NULL);

    th_sirc_proc(sirc);
}

/* This function runs on a separate thread, blocking read a line
 * (terminated with "\r\n") from socket and then pass it to sirc_recv(), which
 * runs on GLib main thread. */
static void th_sirc_proc(SircSession *sirc){
    for (;;){
        th_sirc_recv(sirc);
        if (sirc->fd == -1) {
            DBG_FR("SircSession thread exit");
            return;
        }
    }
}

static void th_sirc_recv(SircSession *sirc){
    g_mutex_lock(&sirc->mutex);
    int ret = sck_readline(sirc->fd, sirc->buf, sizeof(sirc->buf));

    if (ret != SRN_ERR){
        g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                (GSourceFunc)idle_sirc_recv, sirc, NULL);
    } else {
        ERR_FR("Socket error, connection close");
        g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                (GSourceFunc)idle_sirc_on_disconnect, sirc, NULL);
        g_mutex_unlock(&sirc->mutex);
    }
}

/******************************************************************************
 * Idle functions
 *****************************************************************************/

static int idle_sirc_recv(SircSession *sirc){
    IRCMsg imsg;

    int res = irc_parse(sirc->buf, &imsg);

    switch (res){
        case IRCMSG_PING:
            irc_cmd_pong(sirc->fd, sirc->buf);
            sirc->events.ping("PING", sirc->ctx);
            break;
        default:
            irc_event_hdr(sirc, &imsg);
            break;
    }

    g_mutex_unlock(&sirc->mutex);
    return FALSE;
}

static int idle_sirc_on_connect(SircSession *sirc){
    sirc->events.connect("connect", sirc->ctx);
    return FALSE;
}

static int idle_sirc_on_disconnect(SircSession *sirc){
    sirc->events.disconnect("disconnect", sirc->ctx);
    return FALSE;
}
