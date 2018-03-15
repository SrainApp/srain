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

#ifndef __APP_H
#define __APP_H

#include <glib.h>

#include "sui/sui.h"
#include "sirc/sirc.h"
#include "config/config.h"
#include "version.h"

#ifndef __IN_CORE_H
	#error This file should not be included directly, include just core.h
#endif

typedef struct _SrnApplication SrnApplication;
typedef struct _SrnApplicationConfig SrnApplicationConfig;

struct _SrnApplication {
    SrnVersion *ver;
    SrnConfigManager *cfg_mgr;
    SrnApplicationConfig *cfg;

    SrnLogger *logger;
    SrnLoggerConfig *logger_cfg;

    SuiApplication *ui;
    SuiApplicationEvents ui_app_events;
    SuiWindowEvents ui_win_events;
    SuiEvents ui_events;
    SircEvents irc_events;

    GSList *win_list;
    SuiWindowConfig *win_cfg;

    GSList *srv_list;
    GSList *srv_cfg_list;
};

struct _SrnApplicationConfig {
    bool prompt_on_quit; // TODO
    bool create_user_file; // TODO
    char *id;

    SuiApplicationConfig *ui;
    GSList *srv_list; // Predefined server config from config file
};

GType srn_application_get_type(void);
SrnApplication *srn_application_new(void);
SrnApplication* srn_application_get_default(void);
void srn_application_run(SrnApplication *app, int argc, char *argv[]);
void srn_application_quit(SrnApplication *app);

// Server
Server* srn_application_add_server(SrnApplication *app, const char *name);
SrnRet srn_application_rm_server(SrnApplication *app, Server *srv);
Server* srn_application_get_server(SrnApplication *app, const char *name);
bool srn_application_is_server_valid(SrnApplication *app, Server *srv);

// Server config
SrnRet srn_application_add_server_config(SrnApplication *app, ServerPrefs *srv_cfg);
SrnRet srn_application_add_server_config(SrnApplication *app, ServerPrefs *srv_cfg);
SrnRet srn_application_rm_server_config(SrnApplication *app, ServerPrefs *srv_cfg);
ServerPrefs* srn_application_get_server_config(SrnApplication *app, const char *name);
char* srn_application_dump_server_config_list(SrnApplication *app);

SrnApplicationConfig *srn_application_config_new(void);
SrnRet srn_application_config_check(SrnApplicationConfig *cfg);
void srn_application_config_free(SrnApplicationConfig *cfg);

// FIXME: config
SrnRet srn_config_manager_read_log_config(SrnConfigManager *mgr, SrnLoggerConfig *cfg);
SrnRet srn_config_manager_read_application_config(SrnConfigManager *mgr, SrnApplicationConfig *cfg);
SrnRet srn_config_manager_read_server_config_list(SrnConfigManager *mgr, GSList **srv_cfg_list);
SrnRet srn_config_manager_read_server_config(SrnConfigManager *mgr, ServerPrefs *cfg, const char *srv_name);
SrnRet srn_config_manager_read_chat_config(SrnConfigManager *mgr, SrnChatConfig *cfg, const char *srv_name, const char *chat_name);

#endif /* __APP_H */
