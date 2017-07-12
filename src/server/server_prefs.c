/**
 * @file server_prefs.c
 * @brief Server Preference {con,de}structor
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-06-23
 *
 * ServerPrefs ia a structure which store all informations required by a Server.
 * All ServerPrefs are indexed by server_prefs_list and have a unique name.
 *
 */

#include <glib.h>
#include <strings.h>

#include "server.h"

#include "sirc/sirc.h"

#include "prefs.h"
#include "log.h"

static GSList *server_prefs_list = NULL;

static int server_prefs_list_add(ServerPrefs *prefs);
static int server_prefs_list_rm(ServerPrefs *prefs);

static int server_prefs_list_add(ServerPrefs *prefs){
    GSList *lst;
    ServerPrefs *old_prefs;

    lst = server_prefs_list;
    while (lst){
        old_prefs = lst->data;
        if (strcasecmp(prefs->name, old_prefs->name) == 0){
            return SRN_ERR;
        }
        lst = g_slist_next(lst);
    }

    server_prefs_list = g_slist_append(server_prefs_list, prefs);

    return SRN_OK;
}

static int server_prefs_list_rm(ServerPrefs *prefs){
    GSList *lst;

    lst = g_slist_find(server_prefs_list, prefs);
    if (!lst){
        return SRN_ERR;
    }

    server_prefs_list = g_slist_delete_link(server_prefs_list, lst);

    return SRN_OK;
}

/**
 * @brief server_prefs_list_init Generating ServerPrefs from config file
 */
void server_prefs_list_init(){
    char *errmsg;

    errmsg = prefs_read_server_prefs_list();
    if (errmsg){
        ERR_FR("Read server prefs list failed: %s", errmsg);
        g_free(errmsg);
    }
}

ServerPrefs* server_prefs_new(const char *name){
    ServerPrefs *prefs;
    SircPrefs *irc_prefs;

    prefs = g_malloc0(sizeof(ServerPrefs));
    irc_prefs = sirc_prefs_new();

    prefs->name = g_strdup(name);
    prefs->irc = irc_prefs;

    if (server_prefs_list_add(prefs) != SRN_OK){
        server_prefs_free(prefs);
        prefs = NULL;
    }

    return prefs;
}

ServerPrefs* server_prefs_get_prefs(const char *name){
    GSList *lst;
    ServerPrefs *prefs;

    lst = server_prefs_list;

    while (lst){
        prefs = lst->data;
        if (g_ascii_strcasecmp(prefs->name, name) == 0){
            return prefs;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}

bool server_prefs_is_valid(ServerPrefs *prefs){
    /* Whether prefs exists in server_prefs_list? */
    if (!prefs || !g_slist_find(server_prefs_list, prefs)){
            return FALSE;
     }

    return (prefs->name
            && prefs->host
            // && prefs->passwd
            && prefs->encoding
            && prefs->nickname
            && prefs->username
            && prefs->realname
            && prefs->part_message
            && prefs->kick_message
            && prefs->away_message
            && prefs->quit_message
            && prefs->irc);
}

void server_prefs_free(ServerPrefs *prefs){
    /* Remove from list first */
    server_prefs_list_rm(prefs);

    if (prefs->name){
        g_free(prefs->name);
        prefs->name = NULL;
    }

    if (prefs->host){
        g_free(prefs->host);
        prefs->host = NULL;
    }

    if (prefs->passwd){
        g_free(prefs->passwd);
        prefs->passwd = NULL;
    }

    if (prefs->encoding){
        g_free(prefs->encoding);
        prefs->encoding = NULL;
    }

    if (prefs->passwd){
        g_free(prefs->passwd);
        prefs->passwd = NULL;
    }

    if (prefs->encoding){
        g_free(prefs->encoding);
        prefs->encoding = NULL;
    }

    if (prefs->nickname){
        g_free(prefs->nickname);
        prefs->nickname = NULL;
    }

    if (prefs->username){
        g_free(prefs->username);
        prefs->username = NULL;
    }

    if (prefs->realname){
        g_free(prefs->realname);
        prefs->realname = NULL;
    }

    if (prefs->part_message){
        g_free(prefs->part_message);
        prefs->part_message = NULL;
    }

    if (prefs->kick_message){
        g_free(prefs->kick_message);
        prefs->kick_message = NULL;
    }

    if (prefs->away_message){
        g_free(prefs->away_message);
        prefs->away_message = NULL;
    }

    if (prefs->quit_message){
        g_free(prefs->quit_message);
        prefs->quit_message = NULL;
    }

    if (prefs->irc){
        sirc_prefs_free(prefs->irc);
        prefs->irc = NULL;
    }
}
