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
 * @file server_config.c
 * @brief Server config {con,de}structor
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-06-23
 */

#include <glib.h>
#include <strings.h>

#include "core/core.h"
#include "sirc/sirc.h"
#include "config/config.h"
#include "log.h"
#include "utils.h"
#include "i18n.h"

SrnServerConfig* srn_server_config_new(const char *name){
    SrnServerConfig *cfg;
    SircConfig *irc_cfg;

    cfg = g_malloc0(sizeof(SrnServerConfig));
    irc_cfg = sirc_config_new();

    cfg->predefined = FALSE;
    cfg->name = g_strdup(name);
    cfg->irc = irc_cfg;
    cfg->srv = NULL;

    return cfg;
}

SrnServerConfig* srn_server_config_new_from_basename(const char *basename){
    int i = 0;
    SrnServerConfig *cfg;

    cfg = srn_server_config_new(basename);
    while (!cfg && i < 10){
        /* If the name repeats, generates a name for it */
        char *name = g_strdup_printf("%s#%d", basename, ++i);
        cfg = srn_server_config_new(name);
        g_free(name);
    }

    return cfg;
}

SrnRet srn_server_config_check(SrnServerConfig *cfg){
    const char *missing = _("Missing field in SrnServerConfig: %1$s");
    const char *invalid = _("Invalid value in SrnServerConfig: %1$s");

    if (!cfg){
        return RET_ERR(_("Invalid SrnServerConfig instance"));
    }

    if (g_slist_length(cfg->addrs) == 0) {
        return RET_ERR(missing, "addrs");
    }
    for (GSList *lst = cfg->addrs; lst; lst = g_slist_next(lst)){
        SrnServerAddr *addr;

        addr = lst->data;
        if (addr->port == 0) {
            if (cfg->irc->tls) {
                addr->port = 6697;
            } else {
                addr->port = 6667;
            }
        }
    }

    if (str_is_empty(cfg->name)) {
        return RET_ERR(missing, "name");
    }
    if (str_is_empty(cfg->nickname)) {
        return RET_ERR(missing, "nickname");
    }
    if (str_is_empty(cfg->username)) {
        str_assign(&cfg->username, cfg->nickname);
    }
    if (str_is_empty(cfg->realname)) {
        str_assign(&cfg->realname, cfg->nickname);
    }

    switch (cfg->login_method) {
        case LOGIN_NONE:
            break;
        case LOGIN_PASS:
            if (str_is_empty(cfg->passwd)) {
                return RET_ERR(missing, "passwd");
            }
            break;
        case LOGIN_NICKSERV:
        case LOGIN_MSG_NICKSERV:
        case LOGIN_SASL_PLAIN:
            if (str_is_empty(cfg->user_passwd)) {
                return RET_ERR(missing, "user.passwd");
            }
            break;
        case LOGIN_UNKNOWN:
        default:
            return RET_ERR(invalid, "login_method");
    }

    if (str_is_empty(cfg->part_message)) {
        str_assign(&cfg->part_message, "Leaving");
    }
    if (str_is_empty(cfg->kick_message)) {
        str_assign(&cfg->kick_message, "Kick");
    }
    if (str_is_empty(cfg->away_message)) {
        str_assign(&cfg->away_message, "Away");
    }
    if (str_is_empty(cfg->quit_message)) {
        str_assign(&cfg->quit_message, "El Psy Congroo.");
    }

    if (!cfg->irc) {
        return RET_ERR(missing, "irc");
    }

    return sirc_config_check(cfg->irc);
}

void srn_server_config_free(SrnServerConfig *cfg){
    str_assign(&cfg->name, NULL);
    g_slist_free_full(cfg->addrs, (GDestroyNotify)srn_server_addr_free);
    str_assign(&cfg->passwd, NULL);
    str_assign(&cfg->nickname, NULL);
    str_assign(&cfg->username, NULL);
    str_assign(&cfg->realname, NULL);
    str_assign(&cfg->user_passwd, NULL);
    str_assign(&cfg->part_message, NULL);
    str_assign(&cfg->kick_message, NULL);
    str_assign(&cfg->away_message, NULL);
    str_assign(&cfg->quit_message, NULL);

    if (cfg->irc){
        sirc_config_free(cfg->irc);
        cfg->irc = NULL;
    }

    if (cfg->srv){
        server_free(cfg->srv);
        cfg->srv = NULL;
    }
}

void srn_server_config_add_addr(SrnServerConfig *cfg, const char *host, int port){
    cfg->addrs = g_slist_append(cfg->addrs, srn_server_addr_new(host, port));
}

char* srn_server_config_dump(SrnServerConfig *cfg){
    char *passwd;
    char *user_passwd;
    char *login_method;
    char *dump;
    char *irc_dump;
    GString *str;

    g_return_val_if_fail(cfg, NULL);

    if (!str_is_empty(cfg->passwd)){
        passwd = "********";
    } else {
        passwd = _("None");
    }

    if (!str_is_empty(cfg->user_passwd)){
        user_passwd = "********";
    } else {
        user_passwd = _("None");
    }

    login_method = login_method_to_string(cfg->login_method);
    irc_dump = sirc_config_dump(cfg->irc);

    str = g_string_new("");
    g_string_append_printf(str,
            _("*** Server name: %s, Instance: %p\n"
                "\tAddresses: %s, Password: %s\n"
                "\tNickname: %s, Username: %s, Realname: %s\n"
                "\tLogin method: %s, User password: %s\n"
                "\tPart: %s, Kick: %s, Away: %s, Quit: %s\n"
                "\tIRC configuration: %s"),
            cfg->name, cfg->srv,
            /* TODO: cfg->addrs */ "TODO", passwd,
            cfg->nickname, cfg->username, cfg->realname,
            login_method, user_passwd,
            cfg->part_message, cfg->kick_message, cfg->away_message, cfg->quit_message,
            irc_dump);

    g_free(login_method);
    g_free(irc_dump);
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

SrnServerAddr* srn_server_addr_new(const char *host, int port) {
    SrnServerAddr *addr;

    addr = g_malloc0(sizeof(SrnServerAddr));
    str_assign(&addr->host, host);
    addr->port = port;

    return addr;
}

void  srn_server_addr_free(SrnServerAddr *addr) {
    str_assign(&addr->host, NULL);
    g_free(addr);
}
