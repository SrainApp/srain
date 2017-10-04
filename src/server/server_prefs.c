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
 * @file server_prefs.c
 * @brief Server Preference {con,de}structor
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
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
#include "utils.h"
#include "i18n.h"

GSList *server_prefs_list = NULL;

static int server_prefs_list_add(ServerPrefs *prefs);
static int server_prefs_list_rm(ServerPrefs *prefs);

static int server_prefs_list_add(ServerPrefs *prefs){
    GSList *lst;
    ServerPrefs *old_prefs;

    lst = server_prefs_list;
    while (lst){
        old_prefs = lst->data;
        if (g_ascii_strcasecmp(prefs->name, old_prefs->name) == 0){
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

ServerPrefs* server_prefs_new(const char *name){
    ServerPrefs *prefs;
    SircPrefs *irc_prefs;

    prefs = g_malloc0(sizeof(ServerPrefs));
    irc_prefs = sirc_prefs_new();

    prefs->name = g_strdup(name);
    prefs->irc = irc_prefs;
    prefs->srv = NULL;

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

ServerPrefs* server_prefs_get_prefs_by_host_port(const char *host, int port){
    GSList *lst;
    ServerPrefs *prefs;

    lst = server_prefs_list;

    while (lst){
        prefs = lst->data;
        if (g_ascii_strcasecmp(prefs->host, host) == 0 && prefs->port == port){
            return prefs;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}

bool server_prefs_is_valid(ServerPrefs *prefs){
    /* Whether prefs exists in server_prefs_list? */
    return prefs && g_slist_find(server_prefs_list, prefs);
}

bool server_prefs_is_server_valid(Server *srv){
    GSList *lst;
    ServerPrefs *prefs;

    if (!srv) return FALSE;

    lst = server_prefs_list;
    while (lst){
        prefs = lst->data;
        if (prefs->srv == srv){
            return TRUE;
        }

        lst = g_slist_next(lst);
    }

    return FALSE;
}

SrnRet server_prefs_check(ServerPrefs *prefs){
    const char *fmt = _("Missing field in ServerPrefs: %s");

    if (!server_prefs_is_valid(prefs)){
        return RET_ERR(_("Invalid ServerPrefs instance"));
    }

    if (str_is_empty(prefs->name)) {
        return RET_ERR(fmt, "name");
    }

    if (str_is_empty(prefs->host)) {
        return RET_ERR(fmt, "host");
    }

    if (str_is_empty(prefs->passwd)) {
        // Password can be NULL
    }

    if (str_is_empty(prefs->encoding)) {
        str_assign(&prefs->encoding, "UTF-8");
    }

    if (str_is_empty(prefs->nickname)) {
        return RET_ERR(fmt, "nickname");
    }

    if (str_is_empty(prefs->username)) {
        str_assign(&prefs->username, prefs->nickname);
    }

    if (str_is_empty(prefs->realname)) {
        str_assign(&prefs->realname, prefs->nickname);
    }

    if (str_is_empty(prefs->part_message)) {
        str_assign(&prefs->part_message, "Leaving");
    }

    if (str_is_empty(prefs->kick_message)) {
        str_assign(&prefs->kick_message, "Kick");
    }

    if (str_is_empty(prefs->away_message)) {
        str_assign(&prefs->away_message, "Away");
    }

    if (str_is_empty(prefs->quit_message)) {
        str_assign(&prefs->quit_message, "El Psy Congroo.");
    }

    if (!prefs->irc) {
        return RET_ERR(fmt, "irc");
    }

    if (prefs->port == 0) {
        if (prefs->irc->tls) {
            prefs->port = 6697;
        } else {
            prefs->port = 6667;
        }
    }

    return sirc_prefs_check(prefs->irc);
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

    if (prefs->srv){
        server_free(prefs->srv);
        prefs->srv = NULL;
    }
}

char* server_prefs_dump(ServerPrefs *prefs){
    char *passwd;
    char *dump;
    char *irc_dump;
    GString *str;

    g_return_val_if_fail(prefs, NULL);

    if (!str_is_empty(prefs->passwd)){
        passwd = "********";
    } else {
        passwd = NULL;
    }

    irc_dump = sirc_prefs_dump(prefs->irc);

    str = g_string_new("");
    g_string_append_printf(str,
            _("*** Server name: %s, Instance: %p\n"
                "\tHost: %s, Port: %d, Password: %s, Encoding: %s\n"
                "\tNickname: %s, Username: %s, Realname: %s\n"
                "\tPart: %s, Kick: %s, Away: %s, Quit: %s\n"
                "\tIRC configuration: %s"),
            prefs->name, prefs->srv,
            prefs->host, prefs->port, passwd, prefs->encoding,
            prefs->nickname, prefs->username, prefs->realname,
            prefs->part_message, prefs->kick_message, prefs->away_message, prefs->quit_message,
            irc_dump);

    g_free(irc_dump);
    dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

char* server_prefs_list_dump(){
    char *dump;
    GSList *lst;
    GString *str;

    str = g_string_new("");

    lst = server_prefs_list;
    while (lst){
        char *srv_dump;
        ServerPrefs *srv_prefs;

        srv_prefs = lst->data;
        srv_dump = server_prefs_dump(srv_prefs);
        g_string_append_printf(str, _("%s"), srv_dump);
        g_free(srv_dump);

        lst = g_slist_next(lst);
        if (lst) str = g_string_append(str, "\n\n");
    }

    dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}
