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

    prefs->predefined = FALSE;
    prefs->name = g_strdup(name);
    prefs->irc = irc_prefs;
    prefs->srv = NULL;

    if (server_prefs_list_add(prefs) != SRN_OK){
        server_prefs_free(prefs);
        prefs = NULL;
    }

    return prefs;
}

ServerPrefs* server_prefs_new_from_basename(const char *basename){
    int i = 0;
    ServerPrefs *prefs;

    prefs = server_prefs_new(basename);
    while (!prefs && i < 10){
        /* If the name repeats, generates a name for it */
        char *name = g_strdup_printf("%s#%d", basename, ++i);
        prefs = server_prefs_new(name);
        g_free(name);
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
    const char *missing = _("Missing field in ServerPrefs: %1$s");
    const char *invalid = _("Invalid value in ServerPrefs: %1$s");

    if (!server_prefs_is_valid(prefs)){
        return RET_ERR(_("Invalid ServerPrefs instance"));
    }

    if (str_is_empty(prefs->name)) {
        return RET_ERR(missing, "name");
    }

    if (str_is_empty(prefs->host)) {
        return RET_ERR(missing, "host");
    }

    if (str_is_empty(prefs->nickname)) {
        return RET_ERR(missing, "nickname");
    }

    if (str_is_empty(prefs->username)) {
        str_assign(&prefs->username, prefs->nickname);
    }

    if (str_is_empty(prefs->realname)) {
        str_assign(&prefs->realname, prefs->nickname);
    }

    switch (prefs->login_method) {
        case LOGIN_NONE:
            break;
        case LOGIN_PASS:
            if (str_is_empty(prefs->passwd)) {
                return RET_ERR(missing, "passwd");
            }
            break;
        case LOGIN_NICKSERV:
        case LOGIN_MSG_NICKSERV:
        case LOGIN_SASL_PLAIN:
            if (str_is_empty(prefs->user_passwd)) {
                return RET_ERR(missing, "user.passwd");
            }
            break;
        case LOGIN_UNKNOWN:
        default:
            return RET_ERR(invalid, "login_method");
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
        return RET_ERR(missing, "irc");
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
    if (prefs->predefined){
        return;
    }

    /* Remove from list first */
    server_prefs_list_rm(prefs);

    str_assign(&prefs->name, NULL);
    str_assign(&prefs->host, NULL);
    str_assign(&prefs->passwd, NULL);
    str_assign(&prefs->nickname, NULL);
    str_assign(&prefs->username, NULL);
    str_assign(&prefs->realname, NULL);
    str_assign(&prefs->user_passwd, NULL);
    str_assign(&prefs->part_message, NULL);
    str_assign(&prefs->kick_message, NULL);
    str_assign(&prefs->away_message, NULL);
    str_assign(&prefs->quit_message, NULL);

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
    char *user_passwd;
    char *login_method;
    char *dump;
    char *irc_dump;
    GString *str;

    g_return_val_if_fail(prefs, NULL);

    if (!str_is_empty(prefs->passwd)){
        passwd = "********";
    } else {
        passwd = _("None");
    }

    if (!str_is_empty(prefs->user_passwd)){
        user_passwd = "********";
    } else {
        user_passwd = _("None");
    }

    login_method = login_method_to_string(prefs->login_method);
    irc_dump = sirc_prefs_dump(prefs->irc);

    str = g_string_new("");
    g_string_append_printf(str,
            _("*** Server name: %s, Instance: %p\n"
                "\tHost: %s, Port: %d, Password: %s\n"
                "\tNickname: %s, Username: %s, Realname: %s\n"
                "\tLogin method: %s, User password: %s\n"
                "\tPart: %s, Kick: %s, Away: %s, Quit: %s\n"
                "\tIRC configuration: %s"),
            prefs->name, prefs->srv,
            prefs->host, prefs->port, passwd,
            prefs->nickname, prefs->username, prefs->realname,
            login_method, user_passwd,
            prefs->part_message, prefs->kick_message, prefs->away_message, prefs->quit_message,
            irc_dump);

    g_free(login_method);
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
        str = g_string_append(str, srv_dump);
        g_free(srv_dump);

        lst = g_slist_next(lst);
        if (lst) str = g_string_append(str, "\n\n");
    }

    dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

char* login_method_to_string(LoginMethod lm){
    const char *str;

    switch (lm) {
        case LOGIN_NONE:
            str = "none";
            break;
        case LOGIN_PASS:
            str = "pass";
            break;
        case LOGIN_NICKSERV:
            str = "nickserv";
            break;
        case LOGIN_MSG_NICKSERV:
            str = "msg_nickserv";
            break;
        case LOGIN_SASL_PLAIN:
            str = "sasl_plain";
            break;
        case LOGIN_UNKNOWN:
        default:
            str = "unknown";
    }

    return g_strdup(str);
}

LoginMethod login_method_from_string(const char *str){
    LoginMethod login;

    if (str == NULL || g_ascii_strcasecmp(str, "none") == 0){
        login = LOGIN_NONE;
    } else if (g_ascii_strcasecmp(str, "pass") == 0){
        login = LOGIN_PASS;
    } else if (g_ascii_strcasecmp(str, "nickserv") == 0){
        login = LOGIN_NICKSERV;
    } else if (g_ascii_strcasecmp(str, "msg_nickserv") == 0){
        login = LOGIN_MSG_NICKSERV;
    } else if (g_ascii_strcasecmp(str, "sasl_plain") == 0){
        login = LOGIN_SASL_PLAIN;
    } else {
        login = LOGIN_UNKNOWN;
    }

    return login;
}
