/* Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __PASSWORDER_H
#define __PASSWORDER_H

#include <libsecret/secret.h>

#include "core/core.h"
#include "config.h"

/* Synchronous password management functions */

SrnRet srn_config_manager_lookup_server_password(SrnConfigManager *mgr, char **passwd, const char *srv_name);
SrnRet srn_config_manager_store_server_password(SrnConfigManager *mgr, const char *passwd, const char *srv_name);
SrnRet srn_config_manager_clear_server_password(SrnConfigManager *mgr, const char *srv_name);
SrnRet srn_config_manager_lookup_channel_password(SrnConfigManager *mgr, char **passwd, const char *srv_name, const char *chan_name);
SrnRet srn_config_manager_store_channel_password(SrnConfigManager *mgr, const char *passwd, const char *srv_name, const char *chan_name);
SrnRet srn_config_manager_clear_channel_password(SrnConfigManager *mgr, const char *srv_name, const char *chan_name);
SrnRet srn_config_manager_lookup_user_password(SrnConfigManager *mgr, char **passwd, const char *srv_name, const char *user_name);
SrnRet srn_config_manager_store_user_password(SrnConfigManager *mgr, const char *passwd, const char *srv_name, const char *user_name);
SrnRet srn_config_manager_clear_user_password(SrnConfigManager *mgr, const char *srv_name, const char *user_name);

/* For asynchronous password management, please use libsecret interfaces with
 * the following secret schemas and attributes */

#define SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER    "server"
#define SRN_CONFIG_SECRET_SCHEMA_ATTR_CHANNEL   "channel"
#define SRN_CONFIG_SECRET_SCHEMA_ATTR_USER      "user"

SecretSchema* srn_config_manager_get_server_secret_schema(SrnConfigManager *mgr);
SecretSchema* srn_config_manager_get_channel_secret_schema(SrnConfigManager *mgr);
SecretSchema* srn_config_manager_get_user_secret_schema(SrnConfigManager *mgr);

#endif /* __PASSWORDER_H */
