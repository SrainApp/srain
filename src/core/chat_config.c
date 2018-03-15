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

#include <glib.h>

#include "core/core.h"
#include "sui/sui.h"
#include "ret.h"
#include "i18n.h"

SrnChatConfig* srn_chat_config_new(){
    SrnChatConfig *cfg;

    cfg = g_malloc0(sizeof(SrnChatConfig));
    cfg->ui = sui_config_new();

    return cfg;
}

SrnRet srn_chat_config_check(SrnChatConfig *cfg){
    if (!cfg){
        return RET_ERR(_("Invalid chat config instance"));
    }
    return sui_config_check(cfg->ui);
}

void srn_chat_config_free(SrnChatConfig *cfg){
    g_return_if_fail(cfg);

    sui_config_free(cfg->ui);
    g_free(cfg);
}
