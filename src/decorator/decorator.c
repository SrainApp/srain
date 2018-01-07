/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @file decorator.c
 * @brief Decorator is a mechanism for processing XML fromatted message in
 *        flow style
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-05-06
 */

#include <glib.h>

#include "decorator.h"

#include "srain.h"
#include "log.h"
#include "utils.h"

#define MAX_DECORATOR   32  // Bits of a DecoratorFlag(int)

typedef struct _DecoratorContext {
    int index;
    Message *msg;
    Decorator *decorator;
    GString *str;
} DecoratorContext;

static SrnRet do_decorate(DecoratorContext *ctx);
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

extern Decorator relay_decroator;
extern Decorator mirc_colorize_decroator;
extern Decorator mirc_strip_decroator;
extern Decorator pango_markup_decroator;
extern Decorator mention_decroator;
static Decorator *decorators[MAX_DECORATOR];

static GMarkupParser parser = {
    .start_element = start_element,
    .end_element = end_element,
    .text = text,
    .passthrough = passthrough,
    .error = error,
};

void decorator_init(){
    for (int i = 0; i < MAX_DECORATOR; i++){
        decorators[i] = NULL;
    }

    decorators[0] = &relay_decroator;
    decorators[1] = &mirc_strip_decroator;
    decorators[2] = &mirc_colorize_decroator;
    decorators[3] = &pango_markup_decroator;
    decorators[4] = &mention_decroator;
}

/**
 * @brief decorate_message Parse the XML formatted message, pass the plain
 *      text fragment(the text between XML tags) to decorator module.
 *      Decorator module returns the decorated fragment and this function
 *      combines the decorated fragment with the original tags. Finally the
 *      decorated message will be stored in ``msg->dcontent`` and may be passed
 *      to the next decorator modules.
 *
 * @param msg A Message instance, ``msg->dcontent`` should be valid XML which
 *      may without root tag
 * @param flag Indicates which decorator modules to use
 * @param user_data Deprecated
 *
 * @return SRN_OK if success
 *
 * NOTE: As mentioned aboved, decorator module's DecoratorFunc may be called
 * multiple times for single Message instance.
 */
SrnRet decorate_message(Message *msg, DecoratorFlag flag, void *user_data){
    g_return_val_if_fail(msg, SRN_ERR);

    for (int i = 0; i < MAX_DECORATOR; i++){
        DecoratorContext *ctx;

        if (!(flag & (1 << i))
                || !decorators[i]
                || !decorators[i]->name
                || !decorators[i]->func){
            // DBG_FR("No available decorator for bit %d", i);
            continue;
        }

        DBG_FR("Run decorator '%s' for message %p", decorators[i]->name, msg);

        // Set decorator context
        ctx = g_malloc0(sizeof(DecoratorContext));
        ctx->index = 0;
        ctx->msg = msg;
        ctx->decorator = decorators[i];
        ctx->str = g_string_new(NULL);

        do_decorate(ctx);

        g_string_free(ctx->str, TRUE);
        g_free(ctx);
    }

    LOG_FR("Decorated message: %s", msg->dcontent);

    return SRN_OK;
}

static SrnRet do_decorate(DecoratorContext *ctx){
    GError *err;
    GMarkupParseContext *parse_ctx;

    parse_ctx = g_markup_parse_context_new(&parser, 0, ctx, NULL);
    g_markup_parse_context_parse(parse_ctx, "<markup>", -1, NULL);

    err = NULL;
    g_markup_parse_context_parse(parse_ctx, ctx->msg->dcontent, -1, &err);
    if (err){
        ERR_FR("Markup parse error: %s", err->message);
        return SRN_ERR;
    }

    g_markup_parse_context_parse(parse_ctx, "</markup>", -1, NULL);
    g_markup_parse_context_end_parse(parse_ctx, NULL);
    g_markup_parse_context_free(parse_ctx);

    str_assign(&ctx->msg->dcontent, ctx->str->str);

    DBG_FR("Decorator '%s' decorated message: %s",
            ctx->decorator->name, ctx->msg->dcontent);

    return SRN_OK;
}

/* Makeup parser callbacks
 * ref: https://developer.gnome.org/glib/stable/glib-Simple-XML-Subset-Parser.html#GMarkupParser
 */

static void start_element(GMarkupParseContext *context, const gchar *element_name,
        const gchar **attribute_names, const gchar **attribute_values,
        gpointer user_data, GError **error){
    DecoratorContext *ctx = user_data;

    if (g_strcmp0(element_name, "markup") == 0){
        return;
    }

    GString *attr_list = g_string_new(NULL);
    int i = 0;
    while (attribute_names[i] != NULL){
        char *escaped_value = g_markup_escape_text(attribute_values[i], -1);
        g_string_append_printf(attr_list, " %s=\"%s\"",
                attribute_names[i], escaped_value);
        g_free(escaped_value);
        i++;
    }

    DBG_FR("Start tag: %s, attr: %s", element_name, attr_list->str);

    g_string_append_printf(ctx->str, "<%s%s>", element_name, attr_list->str);

    g_string_free(attr_list, TRUE);
}

static void end_element(GMarkupParseContext *context, const gchar *element_name,
        gpointer user_data, GError **error){
    DecoratorContext *ctx = user_data;

    if (g_strcmp0(element_name, "markup") == 0){
        return;
    }

    DBG_FR("End tag: %s", element_name);

    g_string_append_printf(ctx->str, "</%s>", element_name);
}

/* NOTE: text is not nul-terminated */
static void text(GMarkupParseContext *context, const gchar *text, gsize text_len,
        gpointer user_data, GError **error){
    char *frag;
    char *dfrag;
    DecoratorContext *ctx = user_data;

    if (text_len == 0){
        // No text between two xml tags
        return;
    }

    frag = g_strndup(text, text_len);
    DBG_FR("Decorating: index: %d, fragment: %s", ctx->index, frag);

    dfrag = ctx->decorator->func(ctx->msg, ctx->index, frag);
    ctx->index++;
    DBG_FR("Decorated fragment: %s", dfrag);

    if (!dfrag) { // ``ctx->decorator`` doesn't do any change to ``frag``
        dfrag = g_markup_escape_text(frag, -1);
    }

    ctx->str = g_string_append(ctx->str, dfrag);

    g_free(dfrag);
    g_free(frag);
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
