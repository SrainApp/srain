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

/* This is a private header file and should not be exported. */

#ifndef __IN_RENDERER_H
#define __IN_RENDERER_H

#include "core/core.h"

/**
 * @brief SrnMessageRenderer defines a module context of a SrnMessgae rendering
 *module.
 */
typedef struct _SrnMessageRenderer SrnMessageRenderer;

struct _SrnMessageRenderer {
    const char *name;
    void (*init) (void);
    SrnRet (*render) (SrnMessage *msg);
    void (*finalize) (void);
};

#endif /* __IN_RENDERER_H */
