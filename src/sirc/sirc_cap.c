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
 * @file sirc_cap.c
 * @brief IRC client capability negotiation support, version 3.2
 * @author Shengyu Zhang <srain@srain.im>
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

#include "sirc/sirc.h"
#include "log.h"
#include "i18n.h"

static bool sasl_is_support(const char *value);

/* Global cap support table */
static SircCapSupport supported_caps[] = {
    // {
    //     .name = "identify-msg",
    //     .offset = offsetof(SircCap, identify_msg),
    // },

    // /* IRCv3.1 */
    // {
    //     .name = "multi-prefix",
    //     .offset = offsetof(SircCap, mulit_prefix),
    // },
    // {
    //     .name = "away-notify",
    //     .offset = offsetof(SircCap, away_notify),
    // },
    // {
    //     .name = "account-notify",
    //     .offset = offsetof(SircCap, account_notify),
    // },
    // {
    //     .name = "extended-join",
    //     .offset = offsetof(SircCap, extended_join),
    // },
    {
        // Handled manually
        .name = "sasl",
        .offset = offsetof(SircCap, sasl),
        .is_support = sasl_is_support,
    },

    // /* IRCv3.2 */
    // {
    //     .name = "server-time",
    //     .offset = offsetof(SircCap, server_time),
    // },
    // {
    //     .name = "userhost-in-names",
    //     .offset = offsetof(SircCap, userhost_in_names),
    // },
    {
        // Auto enabled on IRCv3.2 and aboved
        .name = "cap-notify",
        .offset = offsetof(SircCap, cap_notify),
    },
    // {
    //     .name = "chghost",
    //     .offset = offsetof(SircCap, chghost),
    // },

    // /* ZNC */
    // {
    //     .name = "znc.in/server-time-iso",
    //     .offset = offsetof(SircCap, znc_server_time_iso),
    // },
    // {
    //     .name = "znc.in/server-time",
    //     .offset = offsetof(SircCap, znc_server_time),
    // },
    {
        // END
        .name = NULL,
    },
};

SircCap* sirc_cap_new(){
    SircCap *scap;

    scap = g_malloc0(sizeof(SircCap));

    return scap;
}

void sirc_cap_free(SircCap *scap){
    g_return_if_fail(scap);

    g_free(scap);
}

SrnRet sirc_cap_enable(SircCap *scap, const char *name, bool enable){
    g_return_val_if_fail(scap, SRN_ERR);

    for (int i = 0; supported_caps[i].name; i++){
        if (g_ascii_strcasecmp(name, supported_caps[i].name) == 0){
            bool *cap = (void *)scap + supported_caps[i].offset;
            *cap = enable;
            return SRN_OK;
        }
    }
    return SRN_ERR;
}

bool sirc_cap_is_support(SircCap *scap, const char *name, const char *value){
    g_return_val_if_fail(scap, FALSE);

    for (int i = 0; supported_caps[i].name; i++){
        if (g_ascii_strcasecmp(name, supported_caps[i].name) == 0){
            return !supported_caps[i].is_support
                || supported_caps[i].is_support(value);
        }
    }
    return FALSE;
}

char* sirc_cap_dump(SircCap *scap){
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
