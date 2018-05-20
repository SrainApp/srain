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
 * @file server_cap.c
 * @brief IRC client capability negotiation support, version 3.2
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.3
 * @date 2017-12-29
 *
 * ref:
 *  - https://ircv3.net/specs/core/capability-negotiation-3.1.html
 *  - https://ircv3.net/specs/core/capability-negotiation-3.2.html
 *  - https://ircv3.net/specs/extensions/cap-notify-3.2.html
 */

#include <string.h>
#include <glib.h>

#include "core/core.h"
#include "srain.h"
#include "log.h"
#include "i18n.h"

typedef struct _ServerCapSupport ServerCapSupport;
struct _ServerCapSupport {
    const char *name;
    ptrdiff_t offset;
    bool (*is_support)(const char *);
    void (*on_enable)(SrnServerCap *, const char *);
};

static bool sasl_is_support(const char *value);
static void sasl_on_enable(SrnServerCap *scap, const char *name);

/* Global cap support table */
static ServerCapSupport supported_caps[] = {
    // {
    //     .name = "identify-msg",
    //     .offset = offsetof(EnabledCap, identify_msg),
    // },

    // /* IRCv3.1 */
    // {
    //     .name = "multi-prefix",
    //     .offset = offsetof(EnabledCap, mulit_prefix),
    // },
    // {
    //     .name = "away-notify",
    //     .offset = offsetof(EnabledCap, away_notify),
    // },
    // {
    //     .name = "account-notify",
    //     .offset = offsetof(EnabledCap, account_notify),
    // },
    // {
    //     .name = "extended-join",
    //     .offset = offsetof(EnabledCap, extended_join),
    // },
    {
        .name = "sasl",
        .offset = offsetof(EnabledCap, sasl),
        .is_support = sasl_is_support,
        .on_enable = sasl_on_enable,
    },

    // /* IRCv3.2 */
    // {
    //     .name = "server-time",
    //     .offset = offsetof(EnabledCap, server_time),
    // },
    // {
    //     .name = "userhost-in-names",
    //     .offset = offsetof(EnabledCap, userhost_in_names),
    // },
    {
        // Auto enabled on IRCv3.2 and aboved
        .name = "cap-notify",
        .offset = offsetof(EnabledCap, cap_notify),
    },
    // {
    //     .name = "chghost",
    //     .offset = offsetof(EnabledCap, chghost),
    // },

    // /* ZNC */
    // {
    //     .name = "znc.in/server-time-iso",
    //     .offset = offsetof(EnabledCap, znc_server_time_iso),
    // },
    // {
    //     .name = "znc.in/server-time",
    //     .offset = offsetof(EnabledCap, znc_server_time),
    // },
    {
        // END
        .name = NULL,
    },
};

SrnServerCap* srn_server_cap_new(){
    SrnServerCap *scap;

    scap = g_malloc0(sizeof(SrnServerCap));

    return scap;
}

void srn_server_cap_free(SrnServerCap *scap){
    g_return_if_fail(scap);

    g_free(scap);
}

SrnRet srn_server_cap_server_enable(SrnServerCap *scap, const char *name, bool enable){
    bool *cap;

    g_return_val_if_fail(scap, SRN_ERR);

    cap = NULL;
    for (int i = 0; supported_caps[i].name; i++){
        if (g_ascii_strcasecmp(name, supported_caps[i].name) == 0){
            cap = (void *)&scap->server_enabled + supported_caps[i].offset;
            *cap = enable;
            break;
        }
    }

    return cap ? SRN_OK : SRN_ERR;
}

SrnRet srn_server_cap_client_enable(SrnServerCap *scap, const char *name, bool enable){
    g_return_val_if_fail(scap, SRN_ERR);

    for (int i = 0; supported_caps[i].name; i++){
        if (g_ascii_strcasecmp(name, supported_caps[i].name) == 0){
            bool *cap;

            cap = (void *)&scap->client_enabled + supported_caps[i].offset;
            *cap = enable;
            if (supported_caps[i].on_enable){
                supported_caps[i].on_enable(scap, name);
            }
            LOG_FR("%s enable: %d", supported_caps[i].name, enable);

            return SRN_OK;
        }
    }

    return SRN_ERR;
}

bool srn_server_cap_all_enabled(SrnServerCap *scap){
    g_return_val_if_fail(scap, FALSE);

    for (int i = 0; supported_caps[i].name; i++){
        bool *server_cap;
        bool *client_cap;

        server_cap = (void *)&scap->server_enabled + supported_caps[i].offset;
        client_cap = (void *)&scap->client_enabled + supported_caps[i].offset;
        if (*server_cap != *client_cap){
            LOG_FR("%s not enabled, server: %d, client: %e",
                    supported_caps[i].name, *server_cap, *client_cap);
            return FALSE;
        }
    }

    return TRUE;
}

bool srn_server_cap_is_support(SrnServerCap *scap, const char *name, const char *value){
    g_return_val_if_fail(scap, FALSE);

    for (int i = 0; supported_caps[i].name; i++){
        if (g_ascii_strcasecmp(name, supported_caps[i].name) == 0){
            bool support;

            support = !supported_caps[i].is_support
                || supported_caps[i].is_support(value);
            LOG_FR("%s support: %d", supported_caps[i].name, support);

            return support;
        }
    }

    return FALSE;
}

char* srn_server_cap_dump(SrnServerCap *scap){
    char *res;
    GString *str;

    g_return_val_if_fail(scap, NULL);

    str = g_string_new(_("Enabled capabilities:"));
    for (int i = 0; supported_caps[i].name; i++){
        bool *cap = (void *)scap + supported_caps[i].offset;
        if (*cap){
            g_string_append_printf(str, " %s", supported_caps[i].name);
        }
    }

    res = str->str;
    g_string_free(str, FALSE);

    return res;
}

static bool sasl_is_support(const char *value){
    bool supported;
    char **mechs;
    const char *supported_mechs[] = { "PLAIN", NULL};

    if (!value) return TRUE;

    supported = FALSE;
    mechs = g_strsplit(value, ",", 0);
    for (int i = 0; mechs[i]; i++){
        for (int j = 0; supported_mechs[j]; j++){
            if (g_ascii_strcasecmp(mechs[i], supported_mechs[j]) == 0){
                DBG_FR("%s is supported", mechs[i]);
                supported = TRUE;
                break;
            }
        }
    }
    g_strfreev(mechs);

    return supported;
}

static void sasl_on_enable(SrnServerCap *scap, const char *name){
    SrnServer *srv;

    srv = scap->srv;
    g_return_if_fail(srv);

    switch (srv->cfg->user->login->method){
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            break;
        default:
            return;
    }
    if (!srv->cap->client_enabled.sasl){
        return;
    }
    if (srv->loggedin){
        return; // TODO: reauth?
    }

    switch (srv->cfg->user->login->method){
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            sirc_cmd_authenticate(srv->irc, "PLAIN");
            break;
        default:
            g_warn_if_reached();
    }
}
