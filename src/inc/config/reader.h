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

#ifndef __READER_H
#define __READER_H

#include "core/core.h"
#include "sui/sui.h"
#include "sirc/sirc.h"
#include "log.h"

SrnRet srn_config_manager_read_log_config(SrnConfigManager *mgr, SrnLoggerConfig *cfg);
SrnRet srn_config_manager_read_application_config(SrnConfigManager *mgr, SrnApplicationConfig *cfg);
SrnRet srn_config_manager_read_server_config_list(SrnConfigManager *mgr, GSList **srv_cfg_list);
SrnRet srn_config_manager_read_server_config(SrnConfigManager *mgr, SrnServerConfig *cfg, const char *srv_name);
SrnRet srn_config_manager_read_server_config_by_addr(SrnConfigManager *mgr, SrnServerConfig *cfg, SrnServerAddr *addr);
SrnRet srn_config_manager_read_chat_config(SrnConfigManager *mgr, SrnChatConfig *cfg, const char *srv_name, const char *chat_name);

#endif /* __READER_H */
