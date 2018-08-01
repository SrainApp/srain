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

#ifndef __SUI_URL_PREVIEWER_H
#define __SUI_URL_PREVIEWER_H

#include <gtk/gtk.h>

#define SUI_TYPE_URL_PREVIEWER (sui_url_previewer_get_type())
#define SUI_URL_PREVIEWER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUI_TYPE_URL_PREVIEWER, SuiUrlPreviewer))
#define SUI_IS_URL_PREVIEWER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SUI_TYPE_URL_PREVIEWER))

typedef enum _SuiUrlContentType SuiUrlContentType;
typedef struct _SuiUrlPreviewer SuiUrlPreviewer;
typedef struct _SuiUrlPreviewerClass SuiUrlPreviewerClass;

enum _SuiUrlContentType {
    SUI_URL_CONTENT_TYPE_UNSUPPORTED,
    SUI_URL_CONTENT_TYPE_TEXT,
    SUI_URL_CONTENT_TYPE_IMAGE,
    SUI_URL_CONTENT_TYPE_UNKNOWN,
};

GType sui_url_previewer_get_type(void);
SuiUrlPreviewer* sui_url_previewer_new(const char *url);
SuiUrlPreviewer* sui_url_previewer_new_from_cache(const char *url);

void sui_url_previewer_preview(SuiUrlPreviewer *self);

const char* sui_url_previewer_get_url(SuiUrlPreviewer *self);
SuiUrlContentType sui_url_previewer_get_content_type(SuiUrlPreviewer *self);

#endif /* __SUI_URL_PREVIEWER_H */
