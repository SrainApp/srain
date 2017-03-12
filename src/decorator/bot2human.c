#define __DBG_ON
#define __LOG_ON

#include <glib.h>

#include "decorator.h"

#include "sirc/sirc.h"

#include "srain.h"
#include "log.h"

GSList* bot_list = NULL;

static int bot2human(Message *msg, DecoratorFlag flag, void *user_data);

Decorator bot2human_decroator = {
    .name = "bot2human",
    .func = bot2human,
};

static int bot2human(Message *msg, DecoratorFlag flag, void *user_data){
    if (!bot_list) {
        LOG_FR("Append");
        bot_list = g_slist_append(bot_list, "teleboto");
        bot_list = g_slist_append(bot_list, "la");
    }

    char *dnick;
    char *dcontent;
    GSList *lst;
    GError *err;
    GRegex *regex;
    GMatchInfo *match_info;

    /* ref: https://github.com/tuna/scripts/blob/master/weechat_bot2human.py#L46 */
    err = NULL;
    regex = g_regex_new("(\x03[0-9,]+)?\\[(?<nick>[^:]+?)\\]\x0f? (?<text>.*)", 0, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed: %s", err->message);
        return SRN_ERR;
    }

    lst = bot_list;
    while (lst){
        if (sirc_nick_cmp(msg->user->nick, lst->data)){
            DBG_FR("Brige bot '%s' found", (char *)lst->data);
            g_regex_match(regex, msg->dcontent, 0, &match_info);

            if (g_match_info_matches(match_info)){
                dnick = g_match_info_fetch_named(match_info, "nick");
                dcontent = g_match_info_fetch_named(match_info, "text");

                g_free(msg->dname);
                g_free(msg->dcontent);
                msg->dname = dnick;
                msg->dcontent = dcontent;

                LOG_FR("Match nick: %s", dnick);
                LOG_FR("Match content: %s", dcontent);
            }

            g_match_info_free(match_info);

        }

        lst = g_slist_next(lst);
    }

    g_regex_unref(regex);

    return SRN_OK;
}

