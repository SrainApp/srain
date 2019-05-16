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

#include <glib.h>

#include "srain.h"
#include "log.h"
#include "utils.h"

#include "render/render.h"
#include "./renderer.h"

// Bits of a SrnRenderFlags(int)
#define MAX_RENDERER   sizeof(SrnRenderFlags) * 8

extern SrnMessageRenderer relay_renderer;
extern SrnMessageRenderer mirc_colorize_renderer;
extern SrnMessageRenderer mirc_strip_renderer;
extern SrnMessageRenderer url_renderer;
extern SrnMessageRenderer mention_renderer;
static SrnMessageRenderer *renderers[MAX_RENDERER];

void srn_render_init(void){
    int i;

    /* NOTE: Do not change the order renderer . */
    i = 0;
    renderers[i++] = &relay_renderer;
    renderers[i++] = &mirc_strip_renderer;
    renderers[i++] = &mirc_colorize_renderer;
    renderers[i++] = &url_renderer;
    renderers[i++] = &mention_renderer;
    g_warn_if_fail(i < MAX_RENDERER);

    /* Initial all renderers */
    for (int i = 0; i < MAX_RENDERER; i++){
        if (!renderers[i] || !renderers[i]->init) {
            continue;
        }
        renderers[i]->init();
    }
}

void srn_render_finalize(void){
    /* Finalize all renderers */
    for (int i = 0; i < MAX_RENDERER; i++){
        if (!renderers[i] || !renderers[i]->finalize) {
            continue;
        }
        renderers[i]->finalize();
    }
}

SrnRet srn_render_message(SrnMessage *msg, SrnRenderFlags flags){
    g_return_val_if_fail(msg, SRN_ERR);

    for (int i = 0; i < MAX_RENDERER; i++){
        if (!(flags & (1 << i))) {
            continue;
        }
        g_return_val_if_fail(renderers[i]
                && renderers[i]->name
                && renderers[i]->render, SRN_ERR);

        SrnRet ret;
        ret = renderers[i]->render(msg);
        if (!RET_IS_OK(ret)) {
            return RET_ERR("Renderer %s failed to render message %p: %s",
                    renderers[i]->name, msg, RET_MSG(ret));
        }
    }

    return SRN_OK;
}
