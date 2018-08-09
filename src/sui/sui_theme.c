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
 * @file sui_theme.c
 * @brief SUI theme management.
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <gtk/gtk.h>

#include "sui_theme.h"

#include "file_helper.h"
#include "log.h"
#include "utils.h"
#include "i18n.h"

struct _SuiThemeManager {
    bool dark;
    char *theme;
    GtkStyleProvider *provider;
    GtkSettings *settings;
};

static void disconnect_gtk_settings(SuiThemeManager *self);
static void connect_gtk_settings(SuiThemeManager *self);
static void on_notify_prefer_dark_theme(GObject *object, GParamSpec *pspec,
        gpointer user_data);

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiThemeManager* sui_theme_manager_new(){
    SuiThemeManager *self;

    self = g_malloc0(sizeof(SuiThemeManager));

    return self;
}

void sui_theme_manager_free(SuiThemeManager *self){
    if (self->settings) {
        disconnect_gtk_settings(self);
    }

    str_assign(&self->theme, NULL);
    if (self->provider) {
        g_object_unref(self->provider);
    }
    g_free(self);
}

SrnRet sui_theme_manager_apply(SuiThemeManager *self, const char *theme){
    char *name;
    char *file;
    GError *err;
    GtkCssProvider *css;
    GdkScreen *screen;
    SrnRet ret;

    if (!self->settings) {
        connect_gtk_settings(self);
    }

    ret = SRN_ERR;
    name = g_strdup_printf("%s%s.css", theme, self->dark ? "-dark" : "");

    file = get_theme_file(name);
    if (!file) {
        ret = RET_ERR("File \"%s\" not found", name);
        goto FIN;
    }

    css = gtk_css_provider_new();
    err = NULL;
    gtk_css_provider_load_from_path(css, file, &err);
    if (err) {
        ret = RET_ERR(_("Failed to apply theme from file \"%1$s\": %2$s"),
                file, err->message);
        g_object_unref(css);
        goto FIN;
    }

    screen = gdk_screen_get_default();

    if (self->provider) { // Clear prevsiou theme
        str_assign(&self->theme, NULL);
        gtk_style_context_remove_provider_for_screen(screen, self->provider);
        g_object_unref(self->provider);
        self->provider = NULL;
    }

    self->provider = GTK_STYLE_PROVIDER(g_object_ref(css));
    gtk_style_context_add_provider_for_screen(screen, self->provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    str_assign(&self->theme, theme);

    ret = SRN_OK;

FIN:
    g_free(name);
    g_free(file);

    return ret;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void connect_gtk_settings(SuiThemeManager *self){
    self->settings = gtk_settings_get_default();
    g_return_if_fail(self->settings);

    g_object_get(self->settings,
            "gtk-application-prefer-dark-theme", &self->dark,
            NULL);

    g_signal_connect(self->settings,
            "notify::gtk-application-prefer-dark-theme",
            G_CALLBACK(on_notify_prefer_dark_theme), self);
}

static void disconnect_gtk_settings(SuiThemeManager *self){
    g_return_if_fail(self->settings);

    g_signal_handlers_disconnect_by_func(
            self->settings,
            on_notify_prefer_dark_theme,
            self);
}

static void on_notify_prefer_dark_theme(GObject *object, GParamSpec *pspec,
        gpointer user_data) {
    char *theme;
    SrnRet ret;
    SuiThemeManager *self;

    self = user_data;

    g_object_get(self->settings,
            "gtk-application-prefer-dark-theme", &self->dark,
            NULL);

    theme = g_strdup(self->theme);
    ret = sui_theme_manager_apply(self, theme);
    g_free(theme);

    if (!RET_IS_OK(ret)) {
        WARN_FR("Failed to change to dark theme: %s", RET_MSG(ret));
    }
}

