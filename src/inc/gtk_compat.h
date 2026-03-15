/* Copyright (C) 2026
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

#ifndef __GTK_COMPAT_H
#define __GTK_COMPAT_H

#include <gtk/gtk.h>

static inline void srn_gtk_widget_add_css_class(GtkWidget *widget,
        const char *css_class){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_add_css_class(widget, css_class);
#else
    GtkStyleContext *style_context;

    style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(style_context, css_class);
#endif
}

static inline void srn_gtk_widget_remove_css_class(GtkWidget *widget,
        const char *css_class){
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_remove_css_class(widget, css_class);
#else
    GtkStyleContext *style_context;

    style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_remove_class(style_context, css_class);
#endif
}

static inline void srn_gtk_image_set_icon_name(GtkImage *image,
        const char *icon_name){
#if GTK_MAJOR_VERSION >= 4
    gtk_image_set_icon_name(image, icon_name);
#else
    gtk_image_set_from_icon_name(image, icon_name, GTK_ICON_SIZE_BUTTON);
#endif
}

#endif /* __GTK_COMPAT_H */
