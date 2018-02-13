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
 * @file theme.c
 * @brief theme
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <gtk/gtk.h>

#include "srain.h"
#include "file_helper.h"

GtkStyleProvider *provider = NULL;

/**
 * @brief theme_load Load a theme by theme name
 *
 * @param theme The name of the theme, for theme file "default.css", the name of
 *              theme is "default"
 *
 * @return SRN_OK if theme loaded
 */
int theme_load(const char *theme){
    char *name;
    char *theme_file;

    name = g_strdup_printf("%s.css", theme);

    theme_file = get_theme_file(name);
    if (!theme_file){
        return SRN_ERR;
    }

    provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
    gtk_css_provider_load_from_path(
            GTK_CSS_PROVIDER(provider), theme_file, NULL);

    g_free(theme_file);
    g_free(name);

    return SRN_OK;
}

#define _G_MAXUINT -1   // YCM can not found the defintion of G_MAXUINT, help him
void theme_apply(GtkWidget *widget){
    if (!provider) return;

    gtk_style_context_add_provider(
            gtk_widget_get_style_context(widget), provider, _G_MAXUINT);

    if (GTK_IS_CONTAINER(widget)){
        gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)theme_apply, NULL);
    }
}
