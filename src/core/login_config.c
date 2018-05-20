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
    str_assign(&self->pass_password, NULL);
    str_assign(&self->nickserv_password, NULL);
    str_assign(&self->msg_nickserv_password, NULL);
    str_assign(&self->sasl_plain_identify, NULL);
    str_assign(&self->sasl_plain_password, NULL);

    g_free(self);
}

SrnRet srn_login_config_check(SrnLoginConfig *self){
    const char *missing = _("Login method %1$s is set while field %2$s is unset");
    const char *unknown = _("Unknown login method");

    switch (self->method) {
        case SRN_LOGIN_METHOD_NONE:
            break;
        case SRN_LOGIN_METHOD_PASS:
            if (str_is_empty(self->pass_password)){
                return RET_ERR(missing, self->method, "pass-password");
            }
            break;
        case SRN_LOGIN_METHOD_NICKSERV:
            if (str_is_empty(self->nickserv_password)){
                return RET_ERR(missing, self->method, "nickserv-password");
            }
            break;
        case SRN_LOGIN_METHOD_MSG_NICKSERV:
            if (str_is_empty(self->msg_nickserv_password)){
                return RET_ERR(missing, self->method, "msg-nickserv-password");
            }
            break;
        case SRN_LOGIN_METHOD_SASL_PLAIN:
            if (str_is_empty(self-> sasl_plain_identify)){
                return RET_ERR(missing, self->method, "sasl-plain-identify");
            }
            if (str_is_empty(self-> sasl_plain_password)){
                return RET_ERR(missing, self->method, "sasl-plain-password");
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
        case SRN_LOGIN_METHOD_PASS:
            str = "pass";
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
    } else if (g_ascii_strcasecmp(str, "pass") == 0){
        login = SRN_LOGIN_METHOD_PASS;
    } else if (g_ascii_strcasecmp(str, "nickserv") == 0){
        login = SRN_LOGIN_METHOD_NICKSERV;
    } else if (g_ascii_strcasecmp(str, "msg-nickserv") == 0){
        login = SRN_LOGIN_METHOD_MSG_NICKSERV;
    } else if (g_ascii_strcasecmp(str, "sasl-plain") == 0){
        login = SRN_LOGIN_METHOD_SASL_PLAIN;
    } else {
        login = SRN_LOGIN_METHOD_UNKNOWN;
    }

    return login;
}
