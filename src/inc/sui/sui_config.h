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

#ifndef __SUI_CONFIG_H
#define __SUI_CONFIG_H

#ifndef __IN_SUI_H
	#error This file should not be included directly, include just sui.h
#endif

#include "srain.h"
#include "ret.h"

typedef struct _SuiApplicationConfig SuiApplicationConfig;
typedef struct _SuiWindowConfig SuiWindowConfig;
typedef struct _SuiBufferConfig SuiBufferConfig;

struct _SuiWindowConfig {
    bool csd;
    bool send_on_ctrl_enter;
    bool exit_on_close;
};

struct _SuiApplicationConfig {
    char *theme;

    SuiWindowConfig window;
};

struct _SuiBufferConfig {
    bool notify;
    bool show_topic;
    bool show_avatar;
    bool show_user_list;
    bool preview_url;
    bool auto_preview_url;
    char *nick_completion_suffix;
};

SuiApplicationConfig *sui_application_config_new(void);
SrnRet sui_application_config_check(SuiApplicationConfig *cfg);
void sui_application_config_free(SuiApplicationConfig *cfg);

SuiWindowConfig *sui_window_config_new(void);
SrnRet sui_window_config_check(SuiWindowConfig *cfg);
void sui_window_config_free(SuiWindowConfig *cfg);

SuiBufferConfig *sui_buffer_config_new(void);
SrnRet sui_buffer_config_check(SuiBufferConfig *cfg);
void sui_buffer_config_free(SuiBufferConfig *cfg);

#endif /* __SUI_CONFIG_H */
