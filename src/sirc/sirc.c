/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file sirc.c
 * @brief Srain IRC library
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-02
 *
 */


#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "sirc/sirc.h"
#include "sirc_parse.h"
#include "sirc_event_hdr.h"

#include "srain.h"
#include "log.h"
#include "i18n.h"

struct _SircSession {
    int bufptr;
    char buf[SIRC_BUF_LEN];
    GSocketClient *client;
    GIOStream *stream;
    GCancellable *cancel;

    SircEvents *events; // Event callbacks
    SircPrefs *prefs;
    void *ctx;
};

static void sirc_recv(SircSession *sirc);

static void on_connect_ready(GObject *obj, GAsyncResult *result, gpointer user_data);
static gboolean on_accept_certificate(GTlsClientConnection *conn,
        GTlsCertificate *cert, GTlsCertificateFlags errors, gpointer user_data);
static void on_connect_fail(SircSession *sirc, const char *reason);
static void on_connect_finish(SircSession *sirc, GIOStream *stream);
static void on_disconnect_ready(GObject *obj, GAsyncResult *result, gpointer user_data);
static void on_recv_ready(GObject *obj, GAsyncResult *res, gpointer user_data);
static void on_disconnect(SircSession *sirc, const char *reason);

SircSession* sirc_new_session(SircEvents *events, SircPrefs *prefs){
    SircSession *sirc;

    g_return_val_if_fail(events, NULL);
    g_return_val_if_fail(prefs, NULL);

    sirc = g_malloc0(sizeof(SircSession));

    sirc->events = events;
    sirc->prefs = prefs;
    /* sirc->bufptr = 0; // via g_malloc0() */
    /* sirc->stream = NULL; // via g_malloc0() */
    sirc->client = g_socket_client_new();
    // g_socket_client_set_timeout(sirc->client, SERVER_PING_INTERVAL);
    sirc->cancel = g_cancellable_new();

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

    g_object_unref(sirc->client);
    g_object_unref(sirc->cancel);

    g_free(sirc);
}

GIOStream* sirc_get_stream(SircSession *sirc){
    g_return_val_if_fail(sirc, NULL);

    return sirc->stream;
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

    g_cancellable_reset(sirc->cancel);
    g_socket_client_connect_to_host_async (sirc->client, host,
            port, sirc->cancel, on_connect_ready, sirc);
}

void sirc_cancel_connect(SircSession *sirc){
    g_return_if_fail(sirc);
    g_return_if_fail(!g_cancellable_is_cancelled(sirc->cancel));

    g_cancellable_cancel(sirc->cancel);
}

void sirc_disconnect(SircSession *sirc){
    g_return_if_fail(sirc);
    g_return_if_fail(sirc->stream);

    g_io_stream_close_async(sirc->stream, 0, NULL, on_disconnect_ready, sirc);
}

static void sirc_recv(SircSession *sirc){
    GInputStream *in;

    in = g_io_stream_get_input_stream(sirc->stream);
    g_input_stream_read_async(in, &sirc->buf[sirc->bufptr], 1, G_PRIORITY_DEFAULT,
            sirc->cancel, on_recv_ready, sirc);
}

static void on_recv_ready(GObject *obj, GAsyncResult *res, gpointer user_data){
    int size;
    GInputStream *in;
    GError *err;
    SircSession *sirc;
    SircMessage *imsg;

    sirc = user_data;

    if (g_io_stream_is_closed(sirc->stream)){
        on_disconnect(sirc, _("Local connection closed"));
        return;
    }

    err = NULL;
    in = G_INPUT_STREAM(obj);
    size = g_input_stream_read_finish(in, res, &err);;
    if (err != NULL){
        on_disconnect(sirc, err->message);
        return;
    }
    if (size == 0){
        on_disconnect(sirc, _("Remote connection closed"));
        return;
    }

    sirc->bufptr++;
    if (sirc->bufptr > sizeof(sirc->buf)){
        WARN_FR("Length of the line exceeds the buffer");
        goto FIN;
    }

    /* Read a line */
    if (!(sirc->bufptr >= 2
            && sirc->buf[sirc->bufptr-2] == '\r'
            && sirc->buf[sirc->bufptr-1] == '\n')){
        goto NOTREADY;
    }

    sirc->buf[sirc->bufptr-2] = '\0';
    sirc->buf[sirc->bufptr-1] = '\0';
    sirc->bufptr -= 2;
    sirc->buf[sirc->bufptr] = '\0';

    DBG_FR("Line: %s", sirc->buf);

    imsg = sirc_parse(sirc->buf);
    if (!imsg){
        ERR_FR("Failed to parse line: %s", sirc->buf);
        goto FIN;
    }

    /* Transcoding */
    sirc_message_transcoding(imsg,
            SRN_ENCODING, sirc->prefs->encoding, SRN_FALLBACK_CHAR);
    /* Handle event */
    sirc_event_hdr(sirc, imsg);

    sirc_message_free(imsg);

FIN:
    /* Clear buffer */
    sirc->bufptr = 0;
    memset(sirc->buf, 0, sizeof(sirc->buf));
NOTREADY:
    sirc_recv(sirc); // Continute receiving
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

    err = NULL;
    g_tls_connection_handshake_finish(tls_conn, res, &err);
    if (err != NULL){
        g_object_unref(tls_conn);
        on_connect_fail(sirc, err->message);
        return;
    }
    LOG_FR("TLS handshake successed");

    on_connect_finish(sirc, G_IO_STREAM(tls_conn));
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
    if (err != NULL){
        on_connect_fail(sirc, err->message);
        return;
    }

    err = NULL;
    addr = g_socket_connection_get_remote_address(conn, &err);
    if (err != NULL){
        ERR_FR("Get remote address : %d, %s", err->code, err->message);
    } else {
        // DBG_FR("Remote address: %");
        // TODO: show remote address
        g_object_unref(addr);
    }

    if (sirc->prefs->tls){
         GIOStream *tls_conn;

         err = NULL;
         tls_conn = g_tls_client_connection_new(G_IO_STREAM(conn), NULL, &err);
         g_object_unref(conn);
         if (err != NULL){
             on_connect_fail(sirc, err->message);
             return;
         }

         if (sirc->prefs->tls_noverify){
             g_tls_client_connection_set_validation_flags(
                     G_TLS_CLIENT_CONNECTION(tls_conn), 0);
         } else {
             g_tls_client_connection_set_validation_flags(
                     G_TLS_CLIENT_CONNECTION(tls_conn), G_TLS_CERTIFICATE_VALIDATE_ALL);
         }

         g_signal_connect(tls_conn, "accept-certificate",
                 G_CALLBACK(on_accept_certificate), NULL);

         /* "CONNECT" event will be triggered after TLS handshake,
          * see `on_handshake_ready` */
         g_tls_connection_handshake_async(G_TLS_CONNECTION(tls_conn),
                     G_PRIORITY_DEFAULT, sirc->cancel, on_handshake_ready, sirc);
     } else {
         on_connect_finish(sirc, G_IO_STREAM(conn));
     }
}

static void on_disconnect_ready(GObject *obj, GAsyncResult *result, gpointer user_data){
    GError *err;
    GIOStream *stream;

    stream = G_IO_STREAM(obj);
    err = NULL;

    // The "DISCONNECT" event will be triggered in on_recv_ready()
    g_io_stream_close_finish(stream, result, &err);
}

static void on_connect_finish(SircSession *sirc, GIOStream *stream){
    LOG_FR("Connected");

    sirc->stream = stream;
    sirc_recv(sirc);

    g_return_if_fail(sirc->events->connect);
    sirc->events->connect(sirc, "CONNECT");
}

static void on_connect_fail(SircSession *sirc, const char *reason){
    const char *params[] = { reason };

    ERR_FR("Connect failed: %s", reason);

    g_return_if_fail(sirc->events->connect_fail);
    sirc->events->connect_fail(sirc, "CONNECT_FAIL", "", params, 1);
}

static void on_disconnect(SircSession *sirc, const char *reason){
    const char *params[] = { reason };

    LOG_FR("Disconnected: %s", reason);

    g_object_unref(sirc->stream);
    sirc->stream = NULL;

    g_return_if_fail(sirc->events->disconnect);
    sirc->events->disconnect(sirc, "DISCONNECT", "", params, 1);
}
