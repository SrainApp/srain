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

/**
 * @file nick.c
 * @brief Nick filter
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-04-19
 */

#include <string.h>
#include <glib.h>

#include "sirc/sirc.h"

#include "core/core.h"

#include "filter.h"

#include "srain.h"
#include "log.h"
#include "i18n.h"

static bool nick(const SrnMessage *msg, const char *content);

Filter nick_filter = {
    .name = "nick",
    .func = nick,
};

static bool nick(const SrnMessage *msg, const char *content){
    return !(msg->sender->is_ignored || msg->sender->srv_user->is_ignored);
}
