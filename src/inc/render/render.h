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
  * @file render.h
  * @brief This header provides a modular mechanism for rendering SrnMessage.
  * @author Shengyu Zhang <i@silverrainz.me>
  * @version 
  * @date 2019-05-14
  */

#ifndef __RENDER_H
#define __RENDER_H

#include "core/core.h"

typedef int SrnRenderFlags;

#define SRN_RENDER_FLAG_RELAY           1 << 0
#define SRN_RENDER_FLAG_MIRC_STRIP      1 << 1
#define SRN_RENDER_FLAG_MIRC_COLORIZE   1 << 2
#define SRN_RENDER_FLAG_URL             1 << 3
#define SRN_RENDER_FLAG_MENTION         1 << 4

void srn_render_init(void);
void srn_render_finalize(void);

/**
 * @brief srn_render_message renders a SrnMessage according to the given flags.
 * Fields of SrnMessage may be changed after rendering.
 *
 * @param msg is a SrnMessage instance.
 * @param flags indicates which render moduele to use.
 *
 * @return SRN_OK if render success.
 */
SrnRet srn_render_message(SrnMessage *msg, SrnRenderFlags flags);

#endif /* __RENDER_H */
