/**
 * @file sirc.c
 * @brief Srain IRC library
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-02
 *
 */

#define __LOG_ON
#define __DBG_ON

#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "sirc/sirc.h"
#include "sirc_parse.h"
#include "sirc_event_hdr.h"
#include "io_stream.h"

#include "server.h"

#include "srain.h"
#include "log.h"

struct _SircSession {
    int fd;
    SircSessionFlag flag;
    int bufptr;
    char buf[SIRC_BUF_LEN];
    GSocketClient *client;
    GIOStream *stream;

    SircEvents *events; // Event callbacks
    SircPrefs *prefs;
    void *ctx;
};

static void sirc_recv(SircSession *sirc);

static void on_connect_ready(GObject *obj, GAsyncResult *result, gpointer user_data);
static gboolean on_accept_certificate(GTlsClientConnection *conn,
        GTlsCertificate *cert, GTlsCertificateFlags errors, gpointer user_data);
static void on_connect_finish(SircSession *sirc);
static void on_disconnect_ready(GObject *obj, GAsyncResult *result, gpointer user_data);
static void on_recv_ready(GObject *obj, GAsyncResult *res, gpointer user_data);
static void on_disconnect(SircSession *sirc, const char *reason);

SircSession* sirc_new_session(SircEvents *events, SircPrefs *prefs){
    SircSession *sirc;

    g_return_val_if_fail(events, NULL);

    sirc = g_malloc0(sizeof(SircSession));

    sirc->fd = -1;
    sirc->events = events;
    sirc->prefs = prefs;
    /* sirc->bufptr = 0; // via g_malloc0() */
    /* sirc->stream = NULL; // via g_malloc0() */
    sirc->client = g_socket_client_new();
    // g_socket_client_set_timeout(sirc->client, SERVER_PING_INTERVAL);

    return sirc;
}

/**
 * @brief sirc_free_session Free a SircSession
 *
 * @param sirc
 *
 * NOTE: This function shouled be called after "DISCONNECT" event, because the IO of
 *       SircSession is asynchronous.
 */
void sirc_free_session(SircSession *sirc){
    g_return_if_fail(sirc);

    if (sirc->client){
        g_object_unref(sirc->client);
        sirc->client = NULL;
    }

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

    g_io_stream_close_async(sirc->stream, 0, NULL, on_disconnect_ready, sirc);
}

static void sirc_recv(SircSession *sirc){
    GInputStream *in;

    g_return_if_fail(G_IS_IO_STREAM(sirc->stream));

    in = g_io_stream_get_input_stream(sirc->stream);
    g_input_stream_read_async(in, &sirc->buf[sirc->bufptr], 1, G_PRIORITY_DEFAULT,
            NULL, on_recv_ready, sirc);
}

static void on_recv_ready(GObject *obj, GAsyncResult *res, gpointer user_data){
    int size;
    GInputStream *in;
    SircSession *sirc;
    SircMessage imsg;

    sirc = user_data;

    g_return_if_fail(G_IS_INPUT_STREAM(obj));
    // g_return_if_fail(G_IS_IO_STREAM(sirc->stream));

    /*
    if (g_io_stream_is_closed(sirc->stream)){
        g_object_unref(sirc->stream);
        sirc->stream = NULL;

        on_disconnect(sirc, NULL);
        return;
    }
    */

    in = G_INPUT_STREAM(obj);
    size = g_input_stream_read_finish(in, res, NULL);
    g_return_if_fail(size == 1);

    sirc->bufptr++;
    if (sirc->bufptr > sizeof(sirc->buf)){
        WARN_FR("Length of the line exceeds the buffer");
        g_return_if_fail(FALSE);
    }

    /* Read a line */
    if (sirc->bufptr >= 2
            && sirc->buf[sirc->bufptr-2] == '\r'
            && sirc->buf[sirc->bufptr-1] == '\n'
            ){
        sirc->buf[sirc->bufptr-2] = '\0';
        sirc->buf[sirc->bufptr-1] = '\0';
        sirc->bufptr -= 2;
        sirc->buf[sirc->bufptr] = '\0';

        if (sirc_parse(sirc->buf, &imsg) == SRN_OK){
            sirc_event_hdr(sirc, &imsg);
        }

        memset(sirc->buf, 0, sizeof(sirc->buf));
        sirc->bufptr = 0;
    }

    sirc_recv(sirc);
}

static gboolean on_accept_certificate(GTlsClientConnection *conn,
        GTlsCertificate *cert, GTlsCertificateFlags errors, gpointer user_data){
    const char *errmsg;

    errmsg = NULL;
    if (errors & G_TLS_CERTIFICATE_UNKNOWN_CA)
        errmsg = "unknown-ca";
    if (errors & G_TLS_CERTIFICATE_BAD_IDENTITY)
        errmsg = "bad-identity";
    if (errors & G_TLS_CERTIFICATE_NOT_ACTIVATED)
        errmsg = "not-activated";
    if (errors & G_TLS_CERTIFICATE_EXPIRED)
        errmsg = "expired";
    if (errors & G_TLS_CERTIFICATE_REVOKED)
        errmsg = "revoked";
    if (errors & G_TLS_CERTIFICATE_INSECURE)
        errmsg = "insecure";

    WARN_FR("Certificate error: %s", errmsg);

    return FALSE;
}

static void on_handshake_ready(GObject *obj, GAsyncResult *res, gpointer user_data){
    GError *err;
    GTlsConnection *tls_conn;
    SircSession *sirc;

    tls_conn = G_TLS_CONNECTION(obj);
    sirc = user_data;

    g_object_unref(sirc->stream);
    sirc->stream = G_IO_STREAM(tls_conn);

    err = NULL;
    if (!g_tls_connection_handshake_finish(tls_conn, res, &err)){
        ERR_FR("TLS handshake failed: %s", err->message);

        g_object_unref(sirc->stream);
        sirc->stream = NULL;

        on_disconnect(sirc, err->message);

        return;
    }

    DBG_FR("TLS handshake successed");

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
        on_disconnect(sirc, err->message);

        return;
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


    if (sirc->prefs->use_ssl){
         GIOStream *tls_conn;

         err = NULL;
         tls_conn = g_tls_client_connection_new(G_IO_STREAM(conn), NULL, &err);
         if (!tls_conn){
             ERR_FR("Could not create TLS connection: %s", err->message);
         }

         if (sirc->prefs->verify_ssl_cert){
             g_tls_client_connection_set_validation_flags(
                     G_TLS_CLIENT_CONNECTION(tls_conn), G_TLS_CERTIFICATE_VALIDATE_ALL);
         } else {
             g_tls_client_connection_set_validation_flags(
                     G_TLS_CLIENT_CONNECTION(tls_conn), 0);
         }

         g_signal_connect(tls_conn, "accept-certificate",
                 G_CALLBACK(on_accept_certificate), NULL);

         g_tls_connection_handshake_async(G_TLS_CONNECTION(tls_conn),
                     G_PRIORITY_DEFAULT, NULL, on_handshake_ready, sirc);
     }

    sirc->stream = G_IO_STREAM(conn);

    /* If use TLS, trigger "CONNECT" event after TLS handshake,
     * see `on_handshake_ready` */
    if (sirc->prefs->use_ssl){
        // ...
    } else {
        on_connect_finish(sirc);
    }
}

static void on_disconnect_ready(GObject *obj, GAsyncResult *result, gpointer user_data){
    GError *err;
    GIOStream *stream;
    SircSession *sirc;

    stream = G_IO_STREAM(obj);
    sirc = user_data;
    err = NULL;

    if (!g_io_stream_close_finish(stream, result, &err)){
        on_disconnect(sirc, err->message);
    } else {
        on_disconnect(sirc, NULL);
    }

    g_object_unref(sirc->stream);
    sirc->stream = NULL;
}

static void on_disconnect(SircSession *sirc, const char *reason){
    sirc->events->disconnect(sirc, "DISCONNECT", "", NULL, 0, reason);
}

static void on_connect_finish(SircSession *sirc){
    sirc->events->connect(sirc, "CONNECT");

    sirc_recv(sirc);
}
