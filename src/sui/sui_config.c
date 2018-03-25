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

#include <glib.h>

#include "sui/sui.h"
#include "i18n.h"
#include "utils.h"

SuiApplicationConfig *sui_application_config_new(void){
    return g_malloc0(sizeof(SuiApplicationConfig));
}

SrnRet sui_application_config_check(SuiApplicationConfig *cfg){
    const char *fmt = _("Missing field in SuiApplicationConfig: %1$s");

    if (str_is_empty(cfg->theme)){
        return RET_ERR(fmt, "theme");
    }

    return SRN_OK;
}

void sui_application_config_free(SuiApplicationConfig *cfg){
    str_assign(&cfg->theme, NULL);
    g_free(cfg);
}

SuiWindowConfig *sui_window_config_new(){
    SuiWindowConfig *cfg;

    cfg = g_malloc0(sizeof(SuiWindowConfig));

    return cfg;
}

SrnRet sui_window_config_check(SuiWindowConfig *cfg){
    g_return_val_if_fail(cfg, SRN_ERR);

    return SRN_OK;
}

void sui_window_config_free(SuiWindowConfig *cfg){
    g_return_if_fail(cfg);

    g_free(cfg);
}

SuiBufferConfig* sui_buffer_config_new(){
    SuiBufferConfig *cfg;

    cfg = g_malloc0(sizeof(SuiBufferConfig));

    return cfg;
}

SrnRet sui_buffer_config_check(SuiBufferConfig *cfg){
    if (!cfg){
        return RET_ERR(_("Invalid SuiBufferConfig instance"));
    }
    return SRN_OK;
}

void sui_buffer_config_free(SuiBufferConfig *cfg){
    g_return_if_fail(cfg);

    g_free(cfg);
}
