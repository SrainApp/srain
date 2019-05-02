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
 * @file app_url.c
 * @brief IRC URL parse and perfrom
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-08-06
 */

#include <glib.h>
#include <libsoup/soup.h>

#include "core/core.h"
#include "config/reader.h"
#include "ret.h"
#include "i18n.h"
#include "log.h"
#include "utils.h"

static SrnRet join_comma_separated_chans(SrnServer *srv, const char *comma_chans);

SrnRet srn_application_open_url(SrnApplication *app, const char *url){
    const char *scheme;
    const char *nick;
    const char *passwd;
    const char *host;
    int port;
    const char *path;
    const char *fragment;
    SoupURI *suri;
    SrnRet ret;
    SrnServer *srv;
    SrnServerConfig *cfg;
    SrnServerAddr *addr;

    ret = SRN_ERR;
    srv = NULL;
    cfg = NULL;
    addr = NULL;

    suri = soup_uri_new(url);
    if (!suri){
        ret = RET_ERR(_("Failed to parse \"%1$s\" as URL"), url);
        goto FIN;
    }

    scheme = soup_uri_get_scheme(suri);
    if (g_ascii_strcasecmp(scheme, "irc") != 0
            && g_ascii_strcasecmp(scheme, "ircs") != 0){
        ret = RET_ERR(_("Unsupported protocol: %1$s"), scheme);
        goto FIN;
    }

    nick = soup_uri_get_user(suri);
    passwd = soup_uri_get_password(suri);
    host = soup_uri_get_host(suri);
    port = soup_uri_get_port(suri);
    path = soup_uri_get_path(suri);
    fragment = soup_uri_get_fragment(suri);

    addr = srn_server_addr_new(host, port);
    // Try looking for server with the same address in server list
    srv = srn_application_get_server_by_addr(app, addr);
    if (!srv) {
        const char *name;
        // Try looking for server config with the same address in config
        cfg = srn_server_config_new();
        ret = srn_config_manager_read_server_config_by_addr(
                app->cfg_mgr, cfg, addr);

        if (!RET_IS_OK(ret)){
            // If not found, just load a default server config from config
            ret = srn_config_manager_read_server_config(app->cfg_mgr, cfg, "");
            if (!RET_IS_OK(ret)){
                goto FIN;
            }

            // Add address for default server config
            (void)srn_server_config_add_addr(cfg, addr);
            addr = NULL; // Ownership changed to server config
        }

        if (!cfg->addrs){
            // This should not happend
            ret = SRN_ERR;
            g_warn_if_reached();
            goto FIN;
        }
        if (!str_is_empty(passwd)){
            str_assign(&cfg->password, passwd);
        }
        if (!str_is_empty(nick)){
            str_assign(&cfg->user->nick, nick);
        }
        if (g_ascii_strcasecmp(scheme, "ircs") == 0){
            cfg->irc->tls = TRUE;
        }

        name = cfg->name ? cfg->name : host;
        ret = srn_application_add_server_with_config(app, name, cfg);
        if (!RET_IS_OK(ret)) {
            ret = RET_ERR(_("Failed to add server \"%1$s\": %2$s"),
                    name, RET_MSG(ret));
            goto FIN;
        }
        cfg = NULL; // Ownership changed to server

        srv = srn_application_get_server(app, name);
        ret = srn_server_connect(srv);
        if (!RET_IS_OK(ret)){
            goto FIN;
        }

        srn_server_wait_until_registered(srv);
    }

    if (!srn_server_is_registered(srv)){
        ret =  RET_ERR(_("Failed to register on server \"%1$s\""), srv->name);
        goto FIN;
    }

    /*  Join channels in URL */
    if (!str_is_empty(path)){
        if (path[0] == '/') {
            path++;    // Skip root of URL path
        }
        ret = join_comma_separated_chans(srv, path);
        if (!RET_IS_OK(ret)) {
            ret = RET_ERR(_("Failed to join channel on server \"%1$s\": %2$s"),
                    srv->name, RET_MSG(ret));
            goto FIN;
        }
    }
    if (!str_is_empty(fragment)){
        ret = join_comma_separated_chans(srv, fragment);
        if (!RET_IS_OK(ret)) {
            ret = RET_ERR(_("Failed to join channel on server \"%1$s\": %2$s"),
                    srv->name, RET_MSG(ret));
            goto FIN;
        }
    }

    ret = SRN_OK;
FIN:
    if (suri){
        soup_uri_free(suri);
    }
    if (addr){
        srn_server_addr_free(addr);
    }
    if (cfg){
        srn_server_config_free(cfg);
    }
    if (!RET_IS_OK(ret) && srv){
        // Do not remove server, let the user handle the failure
    }

    return ret;
}

static SrnRet join_comma_separated_chans(SrnServer *srv, const char *comma_chans){
    char *chans;
    const char *chan;
    SrnRet ret;

    /* TODO: how to distinguish between channel and password if channel doen't
     * start with a '#'?
     */

    ret = SRN_OK;
    chans = g_strdup(comma_chans);
    chan = strtok(chans, ",");

    do {
        if (!str_is_empty(chan)){
            DBG_FR("Get channnel: %s", chan);
            if (!sirc_target_is_channel(srv->irc, chan)){
                char *chan2 = g_strdup_printf("#%s", chan);
                ret = sirc_cmd_join(srv->irc, chan2, NULL);
                g_free(chan2);
            } else {
                ret = sirc_cmd_join(srv->irc, chan, NULL);
            }
            if (!(RET_IS_OK(ret))){
                break;
            }
        }
        chan = strtok(NULL, ",");
    } while (chan);

    g_free(chans);

    return ret;
}
