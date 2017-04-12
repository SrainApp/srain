/**
 * @file sirc.c
 * @brief Srain IRC library
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-02
 *
 */

#define __LOG_ON
#define __DBG_ON

#include <glib.h>
#include <gio/gio.h>

#include "sirc/sirc.h"
#include "sirc_parse.h"
#include "sirc_event_hdr.h"
#include "io_stream.h"

#include "srain.h"
#include "log.h"

struct _SircSession {
    int fd;
    SircSessionFlag flag;
    char buf[SIRC_BUF_LEN];
    GRWLock rwlock;  // Buffer lock
    GThread *thread;
    GSocketClient *client;
    GIOStream *stream;

    SircEvents *events; // Event callbacks
    void *ctx;
};

static void sirc_proc(SircSession *sirc);

/* NOTE: The "th_" prefix means that this function invoked in a seprate thread.
 */
static void th_sirc_proc(SircSession *sirc);
static int th_sirc_recv(SircSession *sirc);

static void on_connect_ready(GObject *obj, GAsyncResult *result, gpointer user_data);
static gboolean on_accept_certificate(GTlsClientConnection *conn,
        GTlsCertificate *cert, GTlsCertificateFlags errors, gpointer user_data);
static void on_connect_finish(SircSession *sirc);
static int on_recv(SircSession *sirc);
static int on_disconnect(SircSession *sirc);

SircSession* sirc_new_session(SircEvents *events, SircSessionFlag flag){
    SircSession *sirc;

    g_return_val_if_fail(events, NULL);

    sirc = g_malloc0(sizeof(SircSession));

    sirc->fd = -1;
    sirc->flag = flag;
    sirc->events = events;
    /* sirc->thread = NULL; // via g_malloc0() */
    /* sirc->stream = NULL; // via g_malloc0() */
    sirc->client = g_socket_client_new();

    g_rw_lock_init(&sirc->rwlock);

    return sirc;
}

void sirc_free_session(SircSession *sirc){
    g_return_if_fail(sirc);

    if (sirc->stream){
        sirc_disconnect(sirc);
    }

    g_object_unref(sirc->client);

    g_free(sirc);
}

int sirc_get_fd(SircSession *sirc){
    g_return_val_if_fail(sirc, -1);

    return sirc->fd;
}

GIOStream* sirc_get_stream(SircSession *sirc){
    g_return_val_if_fail(sirc, NULL);

    return sirc->stream;
}

SircSessionFlag sirc_get_flag(SircSession *sirc){
    /* Don't return SRN_ERR(-1 0xffffffff) */
    g_return_val_if_fail(sirc, 0);

    return sirc->flag;
}

SircEvents *sirc_get_events(SircSession *sirc){
    g_return_val_if_fail(sirc, NULL);

    return sirc->events;
}

void sirc_set_ctx(SircSession *sirc, void *ctx){
    g_return_if_fail(sirc);

    sirc->ctx = ctx;
}

void* sirc_get_ctx(SircSession *sirc){
    g_return_val_if_fail(sirc, NULL);

    return sirc->ctx;
}

void sirc_connect(SircSession *sirc, const char *host, int port){
    g_return_if_fail(sirc);
    g_return_if_fail(host);
    g_return_if_fail(port > 0);

    g_socket_client_connect_to_host_async (sirc->client, host,
            port, NULL, on_connect_ready, sirc);
}

void sirc_disconnect(SircSession *sirc){
    g_return_if_fail(sirc);

    g_object_unref(sirc->stream);
    sirc->stream = NULL;
}

static void sirc_proc(SircSession *sirc){
    sirc->thread = g_thread_new(NULL, (GThreadFunc)th_sirc_proc, sirc);
}

/* This function runs on a separate thread, blocking read a line
 * (terminated with "\r\n") from socket and then pass it to sirc_recv(), which
 * runs on GLib main thread. */
static void th_sirc_proc(SircSession *sirc){
    for (;;){
        if (th_sirc_recv(sirc) == SRN_ERR){
            break;
        }
    }

    g_thread_exit(SRN_OK); // Always SRN_OK?
}

static int th_sirc_recv(SircSession *sirc){
    int ret;

    g_rw_lock_writer_lock(&sirc->rwlock);
    ret = io_stream_readline(sirc->stream, sirc->buf, sizeof(sirc->buf));
    g_rw_lock_writer_unlock(&sirc->rwlock);

    if (ret != -1){
        g_rw_lock_reader_lock(&sirc->rwlock);
        g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                (GSourceFunc)on_recv, sirc, NULL);
    } else {
        /* sirc->stream == NULL means connection closed by sirc_disconnect() */
        if (sirc->stream) {
            g_idle_add_full(G_PRIORITY_HIGH_IDLE,
                    (GSourceFunc)on_disconnect, sirc, NULL);
        }
    }

    return ret;
}

static int on_recv(SircSession *sirc){
    SircMessage imsg;

    if (sirc_parse(sirc->buf, &imsg) == SRN_OK){
        sirc_event_hdr(sirc, &imsg);
    }

    g_rw_lock_reader_unlock(&sirc->rwlock);

    return FALSE;
}

static gboolean on_accept_certificate(GTlsClientConnection *conn,
        GTlsCertificate *cert, GTlsCertificateFlags errors, gpointer user_data){
    WARN_FR("Certificate error, ignore it:");

    if (errors & G_TLS_CERTIFICATE_UNKNOWN_CA)
        WARN_FR("unknown-ca ");
    if (errors & G_TLS_CERTIFICATE_BAD_IDENTITY)
        WARN_FR("bad-identity ");
    if (errors & G_TLS_CERTIFICATE_NOT_ACTIVATED)
        WARN_FR("not-activated ");
    if (errors & G_TLS_CERTIFICATE_EXPIRED)
        WARN_FR("expired ");
    if (errors & G_TLS_CERTIFICATE_REVOKED)
        WARN_FR("revoked ");
    if (errors & G_TLS_CERTIFICATE_INSECURE)
        WARN_FR("insecure ");

    return TRUE;
}

static void on_handshake_ready(GObject *obj, GAsyncResult *res, gpointer user_data){
    GError *err;
    GTlsConnection *tls_conn;
    SircSession *sirc;

    tls_conn = G_TLS_CONNECTION(obj);
    sirc = user_data;

    err = NULL;
    if (!g_tls_connection_handshake_finish(tls_conn, res, &err)){
        ERR_FR("TLS handshake failed: %s", err->message);

        sirc_disconnect(sirc);
        on_disconnect(sirc);

        return;
    }

    DBG_FR("TLS handshake successed");

    g_object_unref(sirc->stream);
    sirc->stream = G_IO_STREAM(tls_conn);

    on_connect_finish(sirc);
}

static void on_connect_ready(GObject *obj, GAsyncResult *res, gpointer user_data){
    GError *err;
    GSocketClient *client;
    GSocketConnection *conn;
    GSocketAddress *addr;
    SircSession *sirc;

    client = G_SOCKET_CLIENT(obj);
    sirc = user_data;
    err = NULL;
    conn = g_socket_client_connect_finish(client, res, &err);

    if (!conn){
        ERR_FR("Connect failed: %d, %s", err->code, err->message);
        on_disconnect(sirc);
    } else {
        DBG_FR("Connected");
    }

    err = NULL;
    addr = g_socket_connection_get_remote_address(conn, &err);

    if (!addr){
        ERR_FR("Get remote address : %d, %s", err->code, err->message);
    } else {
        // DBG_FR("Remote address: %");
        // TODO: show remote address
        g_object_unref(addr);
    }


    if (sirc->flag & SIRC_SESSION_SSL){
         GIOStream *tls_conn;

         err = NULL;
         tls_conn = g_tls_client_connection_new(G_IO_STREAM(conn), NULL, &err);
         if (!tls_conn){
             ERR_FR("Could not create TLS connection: %s", err->message);
         }

         if (sirc->flag & SIRC_SESSION_SSL_NOTVERIFY){
             g_tls_client_connection_set_validation_flags(
                     G_TLS_CLIENT_CONNECTION(tls_conn), 0);
         } else {
             g_tls_client_connection_set_validation_flags(
                     G_TLS_CLIENT_CONNECTION(tls_conn), G_TLS_CERTIFICATE_VALIDATE_ALL);
         }

         g_signal_connect(tls_conn, "accept-certificate",
                 G_CALLBACK(on_accept_certificate), NULL);

         g_tls_connection_handshake_async(G_TLS_CONNECTION(tls_conn),
                     G_PRIORITY_DEFAULT, NULL, on_handshake_ready, sirc);
     }

    sirc->stream = G_IO_STREAM(conn);

    /* If use TLS, toggle "connect" event after TLS handshake,
     * see `on_handshake_ready` */
    if (sirc->flag & SIRC_SESSION_SSL){
        // ...
    } else {
        on_connect_finish(sirc);
    }
}

static int on_disconnect(SircSession *sirc){
    sirc->events->disconnect(sirc, "DISCONNECT");

    return FALSE;
}

static void on_connect_finish(SircSession *sirc){
    sirc->events->connect(sirc, "CONNECT");

    sirc_proc(sirc);
}
