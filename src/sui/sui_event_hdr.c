/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.1
 * @date UNKNOWN
 */

#include "sui_event_hdr.h"

#include "log.h"

/* extern from sui.c */
extern SuiAppEvents *app_events;

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
    [SUI_EVENT_CONNECT] = {
        { .key = "name", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "host", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "port", .fmt = SUI_EVENT_PARAM_INT, },
        { .key = "password", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "nick", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "realname", .fmt = SUI_EVENT_PARAM_STRING, },
        { .key = "tls", .fmt = SUI_EVENT_PARAM_BOOL, },
        { .key = "tls-not-verify", .fmt = SUI_EVENT_PARAM_BOOL, },
    },
    [SUI_EVENT_DISCONNECT] = {
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
};

static SrnRet check_params(SuiEvent event, GVariantDict *params);

SrnRet sui_event_hdr(SuiSession *sui, SuiEvent event, GVariantDict *params){
    SuiEvents *events;

    if (!RET_IS_OK(check_params(event, params))){
        return SRN_ERR;
    }

    /* App events */
    switch (event) {
        case SUI_EVENT_OPEN:
            g_return_val_if_fail(app_events->open, SRN_ERR);
            return app_events->open(event, params);
            break;
        case SUI_EVENT_ACTIVATE:
            g_return_val_if_fail(app_events->activate, SRN_ERR);
            return app_events->activate(event, params);
            break;
        case SUI_EVENT_CONNECT:
            g_return_val_if_fail(app_events->connect, SRN_ERR);
            return app_events->connect(event, params);
            break;
        default:
            break;
    }

    g_return_val_if_fail(sui, SRN_ERR);
    events = sui_get_events(sui);
    g_return_val_if_fail(events, SRN_ERR);

    /* SuiSession events */
    switch (event) {
        case SUI_EVENT_DISCONNECT:
            g_return_val_if_fail(events->disconnect, SRN_ERR);
            return events->disconnect(sui, event, params);
            break;
        case SUI_EVENT_SEND:
            g_return_val_if_fail(events->send, SRN_ERR);
            return events->send(sui, event, params);
            break;
        case SUI_EVENT_JOIN:
            g_return_val_if_fail(events->join, SRN_ERR);
            return events->join(sui, event, params);
            break;
        case SUI_EVENT_PART:
            g_return_val_if_fail(events->part, SRN_ERR);
            return events->part(sui, event, params);
            break;
        case SUI_EVENT_QUERY:
            g_return_val_if_fail(events->query, SRN_ERR);
            return events->query(sui, event, params);
            break;
        case SUI_EVENT_UNQUERY:
            g_return_val_if_fail(events->unquery, SRN_ERR);
            return events->unquery(sui, event, params);
            break;
        case SUI_EVENT_KICK:
            g_return_val_if_fail(events->kick, SRN_ERR);
            return events->kick(sui, event, params);
            break;
        case SUI_EVENT_INVITE:
            g_return_val_if_fail(events->invite, SRN_ERR);
            return events->invite(sui, event, params);
            break;
        case SUI_EVENT_WHOIS:
            g_return_val_if_fail(events->whois, SRN_ERR);
            return events->whois(sui, event, params);
            break;
        case SUI_EVENT_IGNORE:
            g_return_val_if_fail(events->ignore, SRN_ERR);
            return events->ignore(sui, event, params);
            break;
        case SUI_EVENT_CUTOVER:
            g_return_val_if_fail(events->cutover, SRN_ERR);
            return events->cutover(sui, event, params);
            break;
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
