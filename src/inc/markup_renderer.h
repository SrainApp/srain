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

#ifndef __MARKUP_RENDERER_H
#define __MARKUP_RENDERER_H

#include <glib.h>

#include "ret.h"

/**
 * @brief SrnMarkupRenderer is a helper object for conveniently parse and
 * reorganize markup text (XML).
 *
 * SrnMarkupRenderer is backend with GMarkupParser, please refer to:
 * https://developer.gnome.org/glib/stable/glib-Simple-XML-Subset-Parser.html
 */
typedef struct _SrnMarkupRenderer SrnMarkupRenderer;

SrnMarkupRenderer *srn_markup_renderer_new(void);
void srn_markup_renderer_free(SrnMarkupRenderer *self);

/**
 * @brief srn_markup_renderer_render 
 *
 * @param self
 * @param markup_in is pass-in markup text to be rendered.
 * @param markup_out is pass-out rendered markup text, must free it after use.
 * @param user_data is user provided data which can be acessed by
 * srn_markup_renderer_get_user_data().
 *
 * @return SRN_OK if render successes.
 */
SrnRet srn_markup_renderer_render(SrnMarkupRenderer *self,
        const char *markup_in, char **markup_out, void *user_data);

/**
 * @brief srn_markup_renderer_get_markup_parser returns the backend markup parer,
 * the returned parser change be changed to meet the needs of user.
 *
 * @param self is a SrnMarkupRenderer instance created by
 * srn_markup_renderer_new().
 *
 * @return a GMarkupParser.
 */
GMarkupParser* srn_markup_renderer_get_markup_parser(SrnMarkupRenderer *self);

/**
 * @brief srn_markup_renderer_get_markup returns a GString which contains 
 * intermediate result of rendering.
 *
 * @param self is a SrnMarkupRenderer instance created by
 * srn_markup_renderer_new().
 *
 * @return a GString instance, which is held by SrnMarkupRenderer, do not free it.
 *
 * This function should only used in callbacks of GMarkupParser.
 */
GString* srn_markup_renderer_get_markup(SrnMarkupRenderer *self);

/**
 * @brief srn_markup_renderer_get_user_data returns user-provided user data.
 *
 * @param self is a SrnMarkupRenderer instance created by
 * srn_markup_renderer_new().
 *
 * @return a pointer which is the user_data you passed to
 * srn_markup_renderer_render().
 *
 * This function should only used in callbacks of GMarkupParser.
 */
void *srn_markup_renderer_get_user_data(SrnMarkupRenderer *self);

#endif /* __MARKUP_RENDERER_H */
