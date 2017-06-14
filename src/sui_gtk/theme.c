/**
 * @file theme.c
 * @brief theme
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
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

    if (!theme) theme = "default";

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
