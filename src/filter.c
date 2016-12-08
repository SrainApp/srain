/**
 * @file filter.c
 * @brief filter of irc received message
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-10
 */

// #define __DBG_ON
// #define __LOG_ON

#include <string.h>
#include <glib.h>
#include <regex.h>
#include <sys/types.h>

#include "srv_session.h"

#include "meta.h"
#include "log.h"
#include "filter.h"

GList *ignore_list;
GList *relaybot_list;
GList *filter_list;

int disable_filter = 0; //TODO: Add filter_disable/enable

typedef struct {
    char ldelim[10];
    char rdelim[10];
    char nick[NICK_LEN];
} RelaybotInfo;

/* if imsg->nick exist in relaybot_list,
 * TODO: update
 * and imsg->message starts with "<ldeilm><human's nick><rdelim>"
 * let imsg->servername = imsg->nick
 *     imsg->nick = human's nick
 *     remove human's nick from imsg->message
 *
 * example:
 * when "xmppbot" is a relaybot and ldeilm = "[", rdelim = "[ "
 * before:
 *      nickname = "xmppbot"
 *      servername = ""
 *      message = "[Lisa] SilverRain: 没有回忆就去创造回忆，没有道路就去开辟道路"
 *
 * after:
 *      nickname = "Lisa"
 *      servername = "xmppbot"
 *      message = "SilverRain: 没有回忆就去创造回忆，没有道路就去开辟道路"
 *  
 *  if {nick,ldelim,rdelim} no matched, nothing will be changed.
 */
// TODO: nick 的大小被隐式的要求 > 512
void filter_relaybot_trans(const char *orgin_nick, char *nick, char *msg){
    int nick_len;
    char *rdelim_ptr;
    char tmp_msg[512];
    GList *lst;
    RelaybotInfo *info;

    lst = relaybot_list;
    while (lst){
        info = lst->data;

        if (strncmp(info->nick, orgin_nick, NICK_LEN) == 0){
            DBG_FR("Relaybot %s found", info->nick);

            /* left delimiter matched */
            if (strncmp(msg, info->ldelim, strlen(info->ldelim)) == 0){
                DBG_FR("Left delim %s found", info->ldelim);

                rdelim_ptr = strstr(msg, info->rdelim);
                /* right delimiter matched */
                if (rdelim_ptr){
                    nick_len = rdelim_ptr - msg - strlen(info->ldelim);
                    DBG_FR("Right delim %s found, nick len = %d", info->rdelim, nick_len);

                    strncpy(nick,
                            msg + strlen(info->ldelim),
                            (nick_len>NICK_LEN?NICK_LEN:nick_len));

                    strncpy(tmp_msg, rdelim_ptr + strlen(info->rdelim), sizeof(tmp_msg) - 1);
                    tmp_msg[sizeof(tmp_msg) - 1] = 0;
                    strcpy(msg, tmp_msg);

                    return;
                }
            }
        }
        lst = lst->next;
    }

}

/**
 * @brief is_ignore 
 *
 * @param nick
 *
 * @return if return 1, ignore this message
 */
int filter_is_ignore(const char *nick){
    GList *lst;

    lst = ignore_list;
    while (lst){
        if (strncmp(lst->data, nick, NICK_LEN) == 0){
            return 1;
        }
        lst = lst->next;
    }
    return 0;
}

int filter_ignore_list_add(const char *nick){
    GList *lst;

    lst = ignore_list;
    while (lst){
        if (strncmp(lst->data, nick, NICK_LEN) == 0){
            WARN_FR("%s already exist in ignore_list", nick);
            return -1;
        }
        lst = lst->next;
    }

    ignore_list = g_list_append(ignore_list, strdup(nick));
    LOG_FR("Add %s", nick);

    return 0;
}

int filter_ignore_list_rm(const char *nick){
    GList *lst;

    lst = ignore_list;
    while (lst){
        if (strncmp(lst->data, nick, NICK_LEN) == 0){
            g_free(lst->data);
            ignore_list = g_list_remove(ignore_list, lst->data);

            LOG_FR("Remove %s", nick);
            return 0;
        }
        lst = lst->next;
    }

    WARN_FR("%s no found", nick);
    return -1;
}

int filter_relaybot_list_add(const char *nick, const char *ldelim, const char* rdelim){
    GList *lst;
    RelaybotInfo *info;

    lst = relaybot_list;
    while (lst){
        info = lst->data;
        if (strncmp(info->nick, nick, NICK_LEN) == 0){
            WARN_FR("%s already exists in relaybot_list", nick);
            return -1;
        }
        lst = lst->next;
    }

    info = g_malloc0(sizeof(RelaybotInfo));
    strncpy(info->nick, nick, NICK_LEN);
    strncpy(info->ldelim, ldelim, 10);
    strncpy(info->rdelim, rdelim, 10);

    relaybot_list = g_list_append(relaybot_list, info);
    LOG_FR("Add %s ldelim '%s' rdelim '%s'", nick, ldelim, rdelim);

    return 0;
}

int filter_relaybot_list_rm(const char *nick){
    GList *lst;
    RelaybotInfo *info;

    lst = relaybot_list;
    while (lst){
        info = lst->data;
        if (strncmp(info->nick, nick, NICK_LEN) == 0){
            g_free(lst->data);
            relaybot_list = g_list_remove(relaybot_list, lst->data);
            LOG_FR("Remove %s", nick);

            return 0;
        }
        lst = lst->next;
    }

    WARN_FR("%s no found", nick);
    return -1;
}

int filter_filter_add_filter(
        const char *name,
        const char *regex,
        const char *channel_name
){
    GList *lst = filter_list;
    FilterItem *item;

    while(lst){
        item = lst->data;
        if (strncmp(item->name, name, FILTER_NAME_MAX_LEN) == 0){
            WARN_FR("%s already exists in filter_list", name);
            return -1;
        }
        lst = lst->next;
    }

    item = g_malloc0(sizeof(FilterItem));
    if (item == 0){
        ERR_FR("ERR: The memory is exhausted");
        return -1;
    }

    strncpy(item->name, name, FILTER_NAME_MAX_LEN);
    strncpy(item->regex, regex, FILTER_MAX_LEN);
    strncpy(item->channel_name, channel_name, CHAN_LEN);

    filter_list = g_list_append(filter_list, item);
    LOG_FR("Add filter %s, regex: '%s', channel: '%s'", name, regex, channel_name);

    return 0;
}

int filter_filter_remove_filter(const char *filter_name) {
    GList *lst = filter_list;
    FilterItem *filter_item;
    while(lst){
        filter_item = lst->data;
        if(strncmp(filter_name, filter_item->name, FILTER_MAX_LEN) == 0){
            g_free(lst->data);
            filter_list = g_list_remove(filter_list, lst->data);
            LOG_FR("Remove filter %s", filter_name);

            return 0;
        }
        lst = lst->next;
    }
    return -1;
}

int filter_filter_show(){
    GList *lst = filter_list;
    FilterItem *item;

    while(lst){
        item = lst->data;
        //TODO: show all filters on the current buffer
        lst = lst->next;
    }
    return 0;
}

//returns 1 if matched 
int preg_test(const char *regex, const char *string){
    regex_t preg;
    //int convert_result;
    int test_result = 0;

    //Convert regex string to type regex_t
    //TODO: Handle the convert result
    regcomp(&preg, regex, REG_EXTENDED);
    if (regex != NULL && regexec(&preg, string, 0, NULL, 0) == 0){
        test_result = 1;
    }
    regfree(&preg);
    return test_result;
}

/**
 * @param message
 * 
 * @return if return 0, pass and display this message
 */
int filter_filter_check_message(
        const char *channel_name,
        const char *nick_name,
        const char *message
){
    GList *lst;
    FilterItem *item;
    int test_result = 0;

    if (disable_filter == 1){
        return 0;
    }
    lst = filter_list;
    while(lst){
        item = lst->data;

        /* If specified a channel_name or channel_name is * */
        if (strncmp(channel_name, item->channel_name, CHAN_LEN) == 0
            || strncmp("*", item->channel_name, 2) == 0){
            test_result =
                preg_test(item->regex, message) ||
                preg_test(item->regex, nick_name);

            if (test_result != 0){
                return 1;
            }
		}
        if (channel_name == NULL){
            test_result =
                preg_test(item->regex, message) ||
                preg_test(item->regex, nick_name);

            if (test_result != 0){
                return 1;
            }
        }
        lst = lst->next;
    }
    return 0;
}
