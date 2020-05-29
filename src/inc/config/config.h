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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <gio/gio.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>
#include <libsecret/secret.h>

#include "ret.h"
#include "version.h"

typedef struct _SrnConfigManager SrnConfigManager;
typedef struct _SrnSettingsStruct SrnSettingsStruct;

struct _SrnSettingsStruct {
    GSettings *gs_cfg;
    GSettingsBackend *gs_backend;
};

struct _SrnConfigManager {
    SrnVersion *ver; // Compatible version
    SrnSettingsStruct gs_user_cfg;
    SrnSettingsStruct gs_system_cfg;
    SecretSchema *srv_secret_schema;
    SecretSchema *chan_secret_schema;
    SecretSchema *user_secret_schema;
};

SrnConfigManager* srn_config_manager_new(SrnVersion *ver);
void srn_config_manager_free(SrnConfigManager *mgr);
SrnRet srn_config_manager_load_user_config(SrnConfigManager *mgr, const char *file);
SrnRet srn_config_manager_load_system_config(SrnConfigManager *mgr, const char *file);

#endif /*__CONFIG_H */
