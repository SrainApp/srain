
#include <stdio.h>
#include <glib.h>

#include "server.h"

#include "decorator.h"

#include "srain.h"
#include "log.h"

static int mention(Message *msg, DecoratorFlag flag, void *user_data);

Decorator mention_decroator = {
    .name = "mention",
    .func = mention,
};


static int mention(Message *msg, DecoratorFlag flag, void *user_data){
    char *nick;
    char pattern[128];
    GError *err;
    GRegex *regex;
    GMatchInfo *match_info;

    /* Genertate pattern */
    nick = g_regex_escape_string(msg->chat->srv->user->nick, -1);
    snprintf(pattern, sizeof(pattern), "\\b%s\\b", nick);
    g_free(nick);

    DBG_FR("Generated pattern: %s", pattern);

    err = NULL;
    regex = g_regex_new(pattern, G_REGEX_CASELESS, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed: %s", err->message);
        return SRN_ERR;
    }

    if (g_regex_match(regex, msg->dcontent, 0, &match_info)){
        DBG_FR("Matchted");
        msg->mentioned = TRUE;
    }

    g_match_info_free(match_info);
    g_regex_unref(regex);

    return SRN_OK;
}
