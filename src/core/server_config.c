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

SrnServerConfig* srn_server_config_new(){
    SrnServerConfig *cfg;

    cfg = g_malloc0(sizeof(SrnServerConfig));

    cfg->irc = sirc_config_new();;
    cfg->user = srn_user_config_new();

    return cfg;
}

SrnRet srn_server_config_check(SrnServerConfig *cfg){
    const char *missing = _("Missing field in server config: %1$s");
    const char *invalid = _("Invalid value in server config: %1$s");
    SrnRet ret;

    if (!cfg){
        return RET_ERR(_("Invalid server config instance"));
    }

    if (g_list_length(cfg->addrs) == 0) {
        return RET_ERR(missing, "addrs");
    }
    for (GList *lst = cfg->addrs; lst; lst = g_list_next(lst)){
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

    ret = srn_user_config_check(cfg->user);
    if (!RET_IS_OK(ret)) {
        return ret;
    }

    ret = sirc_config_check(cfg->irc);
    if (!RET_IS_OK(ret)) {
        return ret;
    }

    return SRN_OK;
}

void srn_server_config_free(SrnServerConfig *cfg){
    str_assign(&cfg->name, NULL);
    g_list_free_full(cfg->addrs, (GDestroyNotify)srn_server_addr_free);
    str_assign(&cfg->passwd, NULL);
    g_list_free_full(cfg->auto_join_chat_list, g_free);
    g_list_free_full(cfg->auto_run_cmd_list, g_free);

    srn_user_config_free(cfg->user);
    sirc_config_free(cfg->irc);

    g_free(cfg);
}

SrnRet srn_server_config_add_addr(SrnServerConfig *cfg, SrnServerAddr *addr){
    cfg->addrs = g_list_append(cfg->addrs, addr);
    return SRN_OK;
}

/**
 * @brief srn_server_config_clear_addr`` clears all addresses of server config.
 *
 * @param cfg
 * @param addr
 */
void srn_server_config_clear_addr(SrnServerConfig *cfg){
    g_list_free_full(cfg->addrs, (GDestroyNotify)srn_server_addr_free);
    cfg->addrs = NULL;
}

char* srn_server_config_dump(SrnServerConfig *cfg){
    // TODO
    return NULL;
}

SrnServerAddr* srn_server_addr_new(const char *host, int port) {
    SrnServerAddr *addr;

    addr = g_malloc0(sizeof(SrnServerAddr));
    str_assign(&addr->host, host);
    addr->port = port;

    return addr;
}

SrnServerAddr* srn_server_addr_new_from_string(const char *str) {
    int port;
    char *host;
    char *tmp;
    SrnServerAddr *addr;

    host = g_strdup(str);
    tmp = host;
    port = 0;
    tmp = strchr(host, ':');
    if (tmp) {
        *tmp = '\0';
        port = g_ascii_strtoull(tmp+1, NULL, 10);
    }
    addr = srn_server_addr_new(host, port);
    g_free(host);

    return addr;
}

void srn_server_addr_free(SrnServerAddr *addr) {
    str_assign(&addr->host, NULL);
    g_free(addr);
}

/**
 * @brief ``srn_server_addr_equal`` determines if two addresses are equal.
 * Note that ``port == 0`` is used as a wildcard here.
 *
 * @param addr1
 * @param addr2
 *
 * @return TRUE if equal.
 */
bool srn_server_addr_equal(SrnServerAddr *addr1, SrnServerAddr *addr2) {
    return g_ascii_strcasecmp(addr1->host, addr2->host) == 0
        && (addr1->port == addr2->port || addr1->port == 0 || addr2->port == 0);
}
