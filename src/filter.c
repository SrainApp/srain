/**
 * @file filter.c
 * @brief filter of irc received message
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-10
 */

#define __LOG_ON

#include <string.h>
#include <glib.h>
#include "srain_magic.h"
#include "irc.h"
#include "log.h"

GList *ignore_list;
GList *relaybot_list;

typedef struct {
    char ldelim[10];
    char rdelim[10];
    char nick[NICK_LEN];
} relaybot_info_t;

/* if imsg->nick exist in relaybot_list,
 * and imsg->nick d
 * let imsg->servername = imsg->nick
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
static void relaybot_trans(irc_msg_t *imsg){
    int nick_len;
    int max_msg_len;
    char *rdelim_ptr;
    char tmp_msg[512];
    GList *lst;
    relaybot_info_t *info;

    lst = relaybot_list;
    while (lst){
        info = lst->data;

        if (strncmp(info->nick, imsg->nick, NICK_LEN) == 0){
            LOG_FR("relaybot %s found", info->nick);

            /* left delimiter matched */
            if (strncmp(imsg->message, info->ldelim, strlen(info->ldelim)) == 0){
                LOG_FR("left delim %s found", info->ldelim);

                rdelim_ptr = strstr(imsg->message, info->rdelim);
                /* right delimiter matched */
                if (rdelim_ptr){
                    nick_len = rdelim_ptr - imsg->message;
                    max_msg_len = MSG_LEN - (rdelim_ptr + strlen(info->rdelim) - imsg->message);
                    LOG_FR("right delim %s found, nick len = %d", info->rdelim, nick_len);

                    strncpy(imsg->servername, imsg->nick, SERVER_LEN);
                    memset(imsg->nick, 0, NICK_LEN);
                    strncpy(imsg->nick,
                            imsg->message + strlen(info->ldelim),
                            (nick_len>NICK_LEN?NICK_LEN:nick_len) - 1);

                    strncpy(tmp_msg, rdelim_ptr + strlen(info->rdelim), max_msg_len);
                    memset(imsg->message, 0, MSG_LEN);
                    strncpy(imsg->message, tmp_msg, max_msg_len);

                    return;
                }
            }
        }
        lst = lst->next;
    }

}

/**
 * @brief is_ignore 
 * @param nick
 * @return if return 1, ignore this message
 */
static int is_ignore(const char *nick){
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

void filter_init(){

}

int filter_ignore_list_add(const char *nick){
    GList *lst;

    lst = ignore_list;
    while (lst){
        if (strncmp(lst->data, nick, NICK_LEN) == 0){
            ERR_FR("%s already exist in ignore_list", nick);
            return -1;
        }
        lst = lst->next;
    }

    ignore_list = g_list_append(ignore_list, strdup(nick));
    LOG_FR("add %s", nick);

    return 0;
}

int filter_ignore_list_rm(const char *nick){
    GList *lst;

    lst = ignore_list;
    while (lst){
        if (strncmp(lst->data, nick, NICK_LEN) == 0){
            free(lst->data);
            lst = g_list_remove(lst, lst->data);
            ignore_list = lst;

            LOG_FR("remove %s", nick);
            return 0;
        }
        lst = lst->next;
    }

    ERR_FR("%s no found", nick);
    return -1;
}

int filter_relaybot_list_add(const char *nick, char *ldelim, char* rdelim){
    GList *lst;
    relaybot_info_t *info;

    lst = relaybot_list;
    while (lst){
        info = lst->data;
        if (strncmp(info->nick, nick, NICK_LEN) == 0){
            ERR_FR("%s already exist in relaybot_list", nick);
            return -1;
        }
        lst = lst->next;
    }

    info = calloc(1, sizeof(relaybot_info_t));
    strncpy(info->nick, nick, NICK_LEN);
    strncpy(info->ldelim, ldelim, 10);
    strncpy(info->rdelim, rdelim, 10);

    relaybot_list = g_list_append(relaybot_list, info);
    LOG_FR("add %s ldelim '%s' rdelim '%s'", nick, ldelim, rdelim);

    return 0;
}

int filter_relaybot_list_rm(const char *nick){
    GList *lst;
    relaybot_info_t *info;

    lst = relaybot_list;
    while (lst){
        info = lst->data;
        if (strncmp(info->nick, nick, NICK_LEN) == 0){
            free(lst->data);
            lst = g_list_remove(lst, lst->data);
            relaybot_list = lst;
            LOG_FR("remove %s", nick);

            return 0;
        }
        lst = lst->next;
    }

    ERR_FR("%s no found", nick);
    return -1;
}

/**
 * @brief filter 
 * @param imsg a PRIVMSG message
 * @return if return 0, ignore this message in srain_idle().
 *
 * a PRIVMSG message must process via `filter()`,
 * the reslut determines whether the message is ignored
 * and do some transfromation for relaybot message
 *
 * see comments of relaybot_trans() & is_ignore()
 */
int filter(irc_msg_t *imsg){
    relaybot_trans(imsg);

    return !is_ignore(imsg->nick);
    // :(
}
