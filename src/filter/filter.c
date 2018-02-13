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
 * @file filter.c
 * @brief Filter is a mechanism for filtering XML fromatted message in
 *        flow style
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-03-16
 */

#include <glib.h>

#include "filter.h"

#include "srain.h"
#include "log.h"

#define MAX_FILTER   32  // Bits of a FilterFlag(int)

typedef struct _FilterContext {
    const Message *msg;
    Filter *filter;
    GString *str;
} FilterContext;

static bool do_filter(FilterContext *ctx);
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

extern Filter nick_filter;
extern Filter regex_filter;
extern Filter chat_log_filter;

static Filter *filters[MAX_FILTER];

static GMarkupParser parser = {
    .start_element = start_element,
    .end_element = end_element,
    .text = text,
    .passthrough = passthrough,
    .error = error,
};

void filter_init(){
    for (int i = 0; i < MAX_FILTER; i++){
        filters[i] = NULL;
    }

    filters[0] = &nick_filter;
    filters[1] = &regex_filter;
    filters[2] = &chat_log_filter;
}

/**
 * @brief filter_message Converts the XML formatted message to plain text, then
 *      pass the plain text to each filter module, if a filter returns ``FALSE``,
 *      This message will not be passed to the next module.
 *
 * @param msg A Message instance, ``msg->dcontent`` should be valid XML which
 *      may without root tag
 * @param flag Indicates which filter modules to use
 * @param user_data Deprecated
 *
 * @return FALSE if this message should be ignored
 */
bool filter_message(const Message *msg, FilterFlag flag, void *user_data){
    bool ret = TRUE;

    g_return_val_if_fail(msg, FALSE);

    for (int i = 0; i < MAX_FILTER; i++){
        FilterContext *ctx;

        if (!(flag & (1 << i))
                || !filters[i]
                || !filters[i]->name
                || !filters[i]->func){
            // DBG_FR("No available filter for bit %d", i);
            continue;
        }

        DBG_FR("Run filter '%s' for message %p", filters[i]->name, msg);

        ctx = g_malloc0(sizeof(FilterContext));
        ctx->msg = msg;
        ctx->filter = filters[i];
        ctx->str = g_string_new(NULL);

        ret = do_filter(ctx);

        g_string_free(ctx->str, TRUE);
        g_free(ctx);

        if (!ret){
            LOG_FR("Filter '%s' blocked message %p", filters[i]->name, msg);
            break;
        }
    }

    return ret;
}

static bool do_filter(FilterContext *ctx){
    GError *err;
    GMarkupParseContext *parse_ctx;

    parse_ctx = g_markup_parse_context_new(&parser, 0, ctx, NULL);
    g_markup_parse_context_parse(parse_ctx, "<markup>", -1, NULL);

    err = NULL;
    g_markup_parse_context_parse(parse_ctx, ctx->msg->dcontent, -1, &err);
    if (err){
        ERR_FR("Markup parse error: %s", err->message);
        return FALSE;
    }

    g_markup_parse_context_parse(parse_ctx, "</markup>", -1, NULL);
    g_markup_parse_context_end_parse(parse_ctx, NULL);
    g_markup_parse_context_free(parse_ctx);

    return ctx->filter->func(ctx->msg, ctx->str->str);
}


/* Makeup parser callbacks
 * ref: https://developer.gnome.org/glib/stable/glib-Simple-XML-Subset-Parser.html#GMarkupParser
 */

static void start_element(GMarkupParseContext *context, const gchar *element_name,
        const gchar **attribute_names, const gchar **attribute_values,
        gpointer user_data, GError **error){
}

static void end_element(GMarkupParseContext *context, const gchar *element_name,
        gpointer user_data, GError **error){
}

/* NOTE: text is not nul-terminated */
static void text(GMarkupParseContext *context, const gchar *text, gsize text_len,
        gpointer user_data, GError **error){
    FilterContext *ctx = user_data;

    if (text_len == 0){
        // No text between two xml tags
        return;
    }

    ctx->str = g_string_append_len(ctx->str, text, text_len);
}

/* Called for strings that should be re-saved verbatim in this same
 * position, but are not otherwise interpretable.  At the moment
 * this includes comments and processing instructions.
 * NOTE: text is not nul-terminated.
 */
static void passthrough(GMarkupParseContext *context,
        const gchar *passthrough_text, gsize text_len, gpointer user_data,
        GError **error){
    /* Ignore comments for now */
}

static void error(GMarkupParseContext *context, GError *error,
        gpointer user_data){
    ERR_FR("Markup parse error: %s", error->message);
}
