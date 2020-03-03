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

#ifndef _EXTRA_DATA_H
#define _EXTRA_DATA_H

#include <glib.h>

typedef struct _SrnExtraData SrnExtraData;

SrnExtraData* srn_extra_data_new(void);
void srn_extra_data_free(SrnExtraData *self);
void* srn_extra_data_get(SrnExtraData *self, const char *key);
void srn_extra_data_set(SrnExtraData *self, const char *key, void *val,
        GDestroyNotify val_destory_func);

#endif /* __EXTRA_DATA_H */
