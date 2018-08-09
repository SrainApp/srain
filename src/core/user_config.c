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
 * @file user_config.c
 * @brief SrnUserConfig {con,de}structor
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version
 * @date 2018-05-12
 */

#include <glib.h>

#include "core/core.h"

#include "i18n.h"
#include "utils.h"

/**
 * @brief ``srn_user_config_new``
 *
 * @param name
 *
 * @return
 */
SrnUserConfig* srn_user_config_new() {
    SrnUserConfig *self;

    self = g_malloc0(sizeof(SrnUserConfig));
    self->login = srn_login_config_new();

    return self;
}

/**
 * @brief ``srn_user_config_free``
 *
 * @param self
 */
void srn_user_config_free(SrnUserConfig *self){
    str_assign(&self->nick, NULL);
    str_assign(&self->username, NULL);
    str_assign(&self->realname, NULL);

    str_assign(&self->away_message, NULL);
    str_assign(&self->part_message, NULL);
    str_assign(&self->kick_message, NULL);
    str_assign(&self->quit_message, NULL);

    srn_login_config_free(self->login);

    g_free(self);
}

SrnRet srn_user_config_check(SrnUserConfig *self){
    const char *missing = _("Missing field in user config: %1$s");
    SrnRet ret;

    if (str_is_empty(self->nick)){
        return RET_ERR(missing, "nick");
    }
    if (str_is_empty(self->username)){
        str_assign(&self->username, self->nick);
    }
    if (str_is_empty(self->realname)){
        str_assign(&self->realname, self->nick);
    }
    if (str_is_empty(self->realname)){
        str_assign(&self->realname, self->nick);
    }

    if (str_is_empty(self->away_message)){
        str_assign(&self->away_message, "");
    }
    if (str_is_empty(self->part_message)){
        str_assign(&self->part_message, "");
    }
    if (str_is_empty(self->kick_message)){
        str_assign(&self->kick_message, "");
    }
    if (str_is_empty(self->quit_message)){
        str_assign(&self->quit_message, "");
    }

    ret = srn_login_config_check(self->login);
    if (!RET_IS_OK(ret)){
        return ret;
    }

    return SRN_OK;
}
