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

#include <glib.h>

#include "srain.h"
#include "ret.h"

#include "markup_renderer.h"

struct _SrnMarkupRenderer {
    GMarkupParser parser;
    GString *str;
    void *user_data;
    bool is_parsing;
};

#define ROOT_TAG "root"

static void start_element(GMarkupParseContext *context, const gchar *element_name,
        const gchar **attribute_names, const gchar **attribute_values,
        gpointer user_data, GError **error);
static void end_element(GMarkupParseContext *context, const gchar *element_name,
        gpointer user_data, GError **error);
static void text(GMarkupParseContext *context, const gchar *text, gsize text_len,
        gpointer user_data, GError **error);
static void passthrough(GMarkupParseContext *context,
        const gchar *passthrough_text, gsize text_len, gpointer user_data,
        GError **error);
static void error(GMarkupParseContext *context, GError *error,
gpointer user_data);

SrnMarkupRenderer *srn_markup_renderer_new(void) {
    SrnMarkupRenderer *self;

    self = g_malloc0(sizeof(SrnMarkupRenderer));
    self->parser.start_element = start_element;
    self->parser.end_element = end_element;
    self->parser.text = text;
    self->parser.passthrough = passthrough;
    self->parser.error = error;
    self->is_parsing = FALSE;

    return self;
}

void srn_markup_renderer_free(SrnMarkupRenderer *self) {
    g_warn_if_fail(!self->is_parsing);
    g_warn_if_fail(!self->str);
    g_warn_if_fail(!self->user_data);
    g_free(self);
}

SrnRet srn_markup_renderer_render(SrnMarkupRenderer *self,
        const char *markup_in, char **markup_out, void *user_data) {
    GError *err;
    GMarkupParseContext *parse_ctx;
    SrnRet ret;

    g_return_val_if_fail(!self->is_parsing, SRN_ERR);

    self->str = g_string_new(NULL);
    self->user_data = user_data;
    self->is_parsing = TRUE;

    ret = SRN_OK;
    err = NULL;
    parse_ctx = g_markup_parse_context_new(&self->parser, 0, self, NULL);
    g_markup_parse_context_parse(parse_ctx, "<" ROOT_TAG ">", -1, NULL);
    g_markup_parse_context_parse(parse_ctx, markup_in, -1, &err);
    g_markup_parse_context_parse(parse_ctx, "</" ROOT_TAG ">", -1, NULL);
    g_markup_parse_context_end_parse(parse_ctx, NULL);
    g_markup_parse_context_free(parse_ctx);

    if (err){
        ret = RET_ERR("Markup parse error: %s", err->message);
        g_error_free(err);
    }

    if (markup_out) {
        *markup_out = self->str->str;
        g_string_free(self->str, FALSE);
    } else {
        g_string_free(self->str, TRUE);
    }

    self->str = NULL;
    self->user_data = NULL;
    self->is_parsing = FALSE;

    return ret;
}

GMarkupParser* srn_markup_renderer_get_markup_parser(SrnMarkupRenderer *self) {
    return &self->parser;
}

GString* srn_markup_renderer_get_markup(SrnMarkupRenderer *self) {
    g_warn_if_fail(self->is_parsing);
    return self->str;
}

void* srn_markup_renderer_get_user_data(SrnMarkupRenderer *self) {
    g_warn_if_fail(self->is_parsing);
    return self->user_data;
}

/* GLib Markup parser callbacks */

static void start_element(GMarkupParseContext *context, const gchar *element_name,
        const gchar **attribute_names, const gchar **attribute_values,
        gpointer user_data, GError **error){
    SrnMarkupRenderer *self;

    self = user_data;

    // Ignore root tag
    if (g_strcmp0(element_name, ROOT_TAG) == 0){
        return;
    }

    self->str = g_string_append_c(self->str, '<');
    self->str = g_string_append(self->str, element_name);
    while (*attribute_names != NULL){
        char *escaped_value = g_markup_escape_text(*attribute_values, -1);
        g_string_append_printf(self->str, " %s=\"%s\"",
                *attribute_names, escaped_value);
        g_free(escaped_value);
        attribute_names++;
        attribute_values++;
    }
    self->str = g_string_append_c(self->str, '>');
}

static void end_element(GMarkupParseContext *context, const gchar *element_name,
        gpointer user_data, GError **error){
    SrnMarkupRenderer *self;

    self = user_data;

    if (g_strcmp0(element_name, ROOT_TAG) == 0){
        return;
    }

    g_string_append_printf(self->str, "</%s>", element_name);
}

/* NOTE: text is not nul-terminated */
static void text(GMarkupParseContext *context, const gchar *text, gsize text_len,
        gpointer user_data, GError **error){
    char *markup;
    SrnMarkupRenderer *self;

    self = user_data;

    markup = g_markup_escape_text(text, text_len);
    self->str = g_string_append(self->str, markup);
    g_free(markup);
}

/* Called for strings that should be re-saved verbatim in this same
 * position, but are not otherwise interpretable.  At the moment
 * this includes comments and processing instructions.
 */
static void passthrough(GMarkupParseContext *context,
        const gchar *passthrough_text, gsize text_len, gpointer user_data,
        GError **error){
    /* Ignore comments for now */
}

static void error(GMarkupParseContext *context, GError *error,
        gpointer user_data){
    /* No need to deal with error here */
}
