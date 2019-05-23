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

/**
 * @file filter.h
 * @brief This header provides a modular mechanism for filering SrnMessage.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version
 * @date 2019-05-16
 */

#ifndef __FILTER_H
#define __FILTER_H

#include "core/core.h"

typedef int SrnFilterFlags;

#define SRN_FILTER_FLAG_USER        1 << 0
#define SRN_FILTER_FLAG_PATTERN     1 << 1
#define SRN_FILTER_FLAG_LOG         1 << 2

void srn_filter_init(void);
void srn_filter_finalize(void);

/**
 * @brief srn_render_message filters a SrnMessage according to the given flags.
 * Fields of SrnMessage MUST not be changed after filtering.
 *
 * @param msg is a SrnMessage instance.
 * @param flags indicates which filter modueles to use.
 *
 * @return FALSE if the given message should be filtered.
 */
bool srn_filter_message(const SrnMessage *msg, SrnFilterFlags flags);

#endif /* __FILTER_H */
