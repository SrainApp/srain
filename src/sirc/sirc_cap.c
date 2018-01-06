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
    // {
    //     // Handled manually
    //     .name = "sasl",
    //     .offset = offsetof(SircCap, sasl),
    // },

    // /* IRCv3.2 */
    // {
    //     .name = "server-time",
    //     .offset = offsetof(SircCap, server_time),
    // },
    // {
    //     .name = "userhost-in-names",
    //     .offset = offsetof(SircCap, userhost_in_names),
    // },
    // {
    //     // Auto offset on IRCv3.2 and aboved
    //     .name = "cap-notify",
    //     .offset = offsetof(SircCap, cap_notify),
    // },
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

SircCap* sirc_cap_new(SircSession *sirc){
    SircCap *scap;

    scap = g_malloc0(sizeof(SircCap));
    scap->supported = g_memdup(supported_caps, sizeof(supported_caps));

    /* Rebase pointers */
    for (int i = 0; scap->supported[i].name; i++) {
        scap->supported[i].enabled =
            (void *)scap + scap->supported[i].offset;
    }

    scap->irc = sirc;
    scap->ls_end = TRUE;
    scap->list_end = TRUE;

    return scap;
}

void sirc_cap_free(SircCap *scap){
    g_return_if_fail(scap);

    g_free(scap->supported);
    g_free(scap);
}

SrnRet sirc_cap_set(SircCap *scap, const char *name, bool enable){
    g_return_val_if_fail(scap, SRN_ERR);

    for (int i = 0; scap->supported[i].name; i++){
        if (g_ascii_strcasecmp(name, scap->supported[i].name) == 0){
            *(scap->supported[i].enabled) = enable;
            return SRN_OK;
        }
    }
    return SRN_ERR;
}

bool sirc_cap_find(SircCap *scap, const char *name){
    g_return_val_if_fail(scap, FALSE);

    for (int i = 0; scap->supported[i].name; i++){
        if (g_ascii_strcasecmp(name, scap->supported[i].name) == 0){
            return TRUE;
        }
    }
    return FALSE;
}

char* sirc_cap_dump(SircCap *scap){
    char *res;
    GString *str;

    g_return_val_if_fail(scap, NULL);

    str = g_string_new(_("Enabled capabilities:"));
    for (int i = 0; scap->supported[i].name; i++){
        if (*(scap->supported[i].enabled)){
            g_string_append_printf(str, " %s", scap->supported[i].name);
        }
    }

    res = str->str;
    g_string_free(str, FALSE);

    return res;
}
