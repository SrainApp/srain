/* Copyright (C) 2016-2020 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __SIRC_COMMAND_BUILDER_H
#define __SIRC_COMMAND_BUILDER_H

#include "srain.h"

/**
 * @brief A helper for building legal length IRC command.
 */
typedef struct _SircCommandBuilder SircCommandBuilder;

SircCommandBuilder* sirc_command_builder_new(const char *cmd);
void sirc_command_builder_free(SircCommandBuilder *self);
bool sirc_command_builder_add_middle(SircCommandBuilder *self, const char *param);
const char* sirc_command_builder_set_trailing(SircCommandBuilder *self, const char *param);
char* sirc_command_builder_build(SircCommandBuilder *self);

#endif /* __SIRC_COMMAND_BUILDER_H */
