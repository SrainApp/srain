/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
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
 * @file login_config.c
 * @brief SrnLoginConfig {con,de}structor
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version
 * @date 2018-05-12
 */

#include <glib.h>

#include "core/core.h"

#include "i18n.h"
#include "utils.h"

SrnLoginConfig* srn_login_config_new(void){
    SrnLoginConfig *self;

    self = g_malloc0(sizeof(SrnLoginConfig));
    self->method = SRN_LOGIN_METHOD_NONE;

    return self;
}

void srn_login_config_free(SrnLoginConfig *self){
    str_assign(&self->password, NULL);
    str_assign(&self->cert_file, NULL);

    g_free(self);
}

SrnRet srn_login_config_check(SrnLoginConfig *self){
    const char *missing =
        _("You need to set the field %2$s in order to use login method %1$s");
    const char *unknown = _("Unknown login method");
    const char *method_str = srn_login_method_to_string(self->method);

    switch (self->method) {
        case SRN_LOGIN_METHOD_NONE:
            break;
        case SRN_LOGIN_METHOD_NICKSERV:
        case SRN_LOGIN_METHOD_MSG_NICKSERV:
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            if (!self->password){
                return RET_ERR(missing, method_str, "password");
            }
            break;
        case SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE:
            if (str_is_empty(self->cert_file)){
                return RET_ERR(missing, method_str, "cerficiate");
            }
            break;
        case SRN_LOGIN_METHOD_UNKNOWN:
        default:
            return RET_ERR(unknown);
    }

    return SRN_OK;
}

const char* srn_login_method_to_string(SrnLoginMethod lm){
    const char *str;

    switch (lm) {
        case SRN_LOGIN_METHOD_NONE:
            str = "none";
            break;
        case SRN_LOGIN_METHOD_NICKSERV:
            str = "nickserv";
            break;
        case SRN_LOGIN_METHOD_MSG_NICKSERV:
            str = "msg-nickserv";
            break;
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            str = "sasl-plain";
            break;
        case SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE:
            str = "sasl-ecdsa-nist256p-challenge";
            break;
        case SRN_LOGIN_METHOD_UNKNOWN:
        default:
            str = "unknown";
    }

    return str;
}

SrnLoginMethod srn_login_method_from_string(const char *str){
    SrnLoginMethod login;

    if (str == NULL || g_ascii_strcasecmp(str, "none") == 0){
        login = SRN_LOGIN_METHOD_NONE;
    } else if (g_ascii_strcasecmp(str, "nickserv") == 0){
        login = SRN_LOGIN_METHOD_NICKSERV;
    } else if (g_ascii_strcasecmp(str, "msg-nickserv") == 0){
        login = SRN_LOGIN_METHOD_MSG_NICKSERV;
    } else if (g_ascii_strcasecmp(str, "sasl-plain") == 0){
        login = SRN_LOGIN_METHOD_SASL_PLAIN;
    } else if (g_ascii_strcasecmp(str, "sasl-ecdsa-nist256p-challenge") == 0){
        login = SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE;
    } else if (g_ascii_strcasecmp(str, "sasl-ecdsa") == 0){ // Shorter alias
        login = SRN_LOGIN_METHOD_SASL_ECDSA_NIST256P_CHALLENGE;
    } else {
        login = SRN_LOGIN_METHOD_UNKNOWN;
    }

    return login;
}
