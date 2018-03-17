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
 * @file sui_event_hdr.c
 * @brief Sui event handler
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date UNKNOWN
 */

#include "sui_event_hdr.h"

#include "log.h"

#define SUI_EVENT_MAX_PARAM     20

typedef struct {
    const char *key;
    const char *fmt;
} SuiEventParamFormat;

static SuiEventParamFormat formats[SUI_EVENT_UNKNOWN][SUI_EVENT_MAX_PARAM] = {
    [SUI_EVENT_OPEN] = {
        { .key = "urls",    .fmt = SUI_EVENT_PARAM_STRINGS, },
        { .key = NULL,      .fmt = NULL, },
    },
    [SUI_EVENT_ACTIVATE] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_SHUTDOWN] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_CONNECT] = {
        { .key = "name", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "host", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "port", .fmt = SUI_EVENT_PARAM_INT, },
        { .key = "password", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "realname", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "tls", .fmt = SUI_EVENT_PARAM_BOOL, },
        { .key = "tls-noverify", .fmt = SUI_EVENT_PARAM_BOOL, },
    },
    [SUI_EVENT_DISCONNECT] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_QUIT] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_SEND] = {
        { .key = "message", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_JOIN] = {
        { .key = "channel", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "password", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_PART] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_QUERY] = {
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_UNQUERY] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_KICK] = {
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_INVITE] = {
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_WHOIS] = {
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_IGNORE] = {
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_CUTOVER] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_SERVER_LIST] = {
        { .key = NULL, .fmt = NULL, },
    },
    [SUI_EVENT_CHAN_LIST] = {
        { .key = NULL, .fmt = NULL, },
    },
};

static SrnRet check_params(SuiEvent event, GVariantDict *params);

SrnRet sui_application_event_hdr(SuiApplication *app, SuiEvent event,
        GVariantDict *params){
    SuiApplicationEvents *events;

    DBG_FR("event: %d", event);

    if (!RET_IS_OK(check_params(event, params))){
        return SRN_ERR;
    }

    g_return_val_if_fail(app, SRN_ERR);
    events = sui_application_get_events(app);
    g_return_val_if_fail(events, SRN_ERR);

    switch (event) {
        case SUI_EVENT_OPEN:
            g_return_val_if_fail(events->open, SRN_ERR);
            return events->open(app, event, params);
        case SUI_EVENT_ACTIVATE:
            g_return_val_if_fail(events->activate, SRN_ERR);
            return events->activate(app, event, params);
        case SUI_EVENT_SHUTDOWN:
            g_return_val_if_fail(events->shutdown, SRN_ERR);
            return events->shutdown(app, event, params);
        default:
            ERR_FR("No such SuiEvent: %d", event);
            return SRN_ERR;
    }
}

SrnRet sui_window_event_hdr(SuiWindow *win, SuiEvent event,
        GVariantDict *params){
    SuiWindowEvents *events;

    DBG_FR("event: %d", event);

    if (!RET_IS_OK(check_params(event, params))){
        return SRN_ERR;
    }

    g_return_val_if_fail(win, SRN_ERR);
    events = sui_window_get_events(win);
    g_return_val_if_fail(events, SRN_ERR);

    switch (event) {
        case SUI_EVENT_CONNECT:
            g_return_val_if_fail(events->connect, SRN_ERR);
            return events->connect(win, event, params);
        case SUI_EVENT_SERVER_LIST:
            g_return_val_if_fail(events->server_list, SRN_ERR);
            return events->server_list(win, event, params);
        default:
            ERR_FR("No such SuiEvent: %d", event);
            return SRN_ERR;
    }
}

SrnRet sui_buffer_event_hdr(SuiBuffer *buf, SuiEvent event, GVariantDict *params){
    SuiBufferEvents *events;

    DBG_FR("event: %d", event);

    if (!RET_IS_OK(check_params(event, params))){
        return SRN_ERR;
    }

    g_return_val_if_fail(buf, SRN_ERR);
    events = sui_buffer_get_events(buf);
    g_return_val_if_fail(events, SRN_ERR);

    /* SuiBuffer events */
    switch (event) {
        case SUI_EVENT_DISCONNECT:
            g_return_val_if_fail(events->disconnect, SRN_ERR);
            return events->disconnect(buf, event, params);
        case SUI_EVENT_QUIT:
            g_return_val_if_fail(events->quit, SRN_ERR);
            return events->quit(buf, event, params);
        case SUI_EVENT_SEND:
            g_return_val_if_fail(events->send, SRN_ERR);
            return events->send(buf, event, params);
        case SUI_EVENT_JOIN:
            g_return_val_if_fail(events->join, SRN_ERR);
            return events->join(buf, event, params);
        case SUI_EVENT_PART:
            g_return_val_if_fail(events->part, SRN_ERR);
            return events->part(buf, event, params);
        case SUI_EVENT_QUERY:
            g_return_val_if_fail(events->query, SRN_ERR);
            return events->query(buf, event, params);
        case SUI_EVENT_UNQUERY:
            g_return_val_if_fail(events->unquery, SRN_ERR);
            return events->unquery(buf, event, params);
        case SUI_EVENT_KICK:
            g_return_val_if_fail(events->kick, SRN_ERR);
            return events->kick(buf, event, params);
        case SUI_EVENT_INVITE:
            g_return_val_if_fail(events->invite, SRN_ERR);
            return events->invite(buf, event, params);
        case SUI_EVENT_WHOIS:
            g_return_val_if_fail(events->whois, SRN_ERR);
            return events->whois(buf, event, params);
        case SUI_EVENT_IGNORE:
            g_return_val_if_fail(events->ignore, SRN_ERR);
            return events->ignore(buf, event, params);
        case SUI_EVENT_CUTOVER:
            g_return_val_if_fail(events->cutover, SRN_ERR);
            return events->cutover(buf, event, params);
        case SUI_EVENT_CHAN_LIST:
            g_return_val_if_fail(events->chan_list, SRN_ERR);
            return events->chan_list(buf, event, params);
        default:
            ERR_FR("No such SuiEvent: %d", event);
            return SRN_ERR;
    }
}

static SrnRet check_params(SuiEvent event, GVariantDict *params){
    SrnRet ret = SRN_OK;

    if (event >= SUI_EVENT_UNKNOWN) {
        ERR_FR("No such SuiEvent: %d", event);
        return SRN_ERR;
    }

    int i = 0;
    SuiEventParamFormat *pfmt;
    for (;;) {
        pfmt = &formats[event][i++];
        if (pfmt->key == NULL) break;

        void *p;
        if (!g_variant_dict_lookup(params, pfmt->key, pfmt->fmt, (void *)&p)){
            ret = SRN_ERR;
            ERR_FR("Params of SuiEvent %d doesn't have key '%s' with format %s",
                    event, pfmt->key, pfmt->fmt);
        }
    }

    return ret;
}
