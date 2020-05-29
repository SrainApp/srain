/* Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
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

#include <glib.h>
#include <string.h>

#include "config/config.h"
#include "config/password.h"
#include "srain.h"
#include "ret.h"
#include "meta.h"
#include "i18n.h"

void srn_config_manager_init_secret_schema(SrnConfigManager *mgr){
    static SecretSchema srv_secret_schema = {
        .name = PACKAGE_APPID ".ServerPassword",
        .flags = SECRET_SCHEMA_NONE,
        .attributes = {
            {
                .name = SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER,
                .type = SECRET_SCHEMA_ATTRIBUTE_STRING,
            },
            { .name = "NULL", .type = 0 },
        }
    };
    mgr->srv_secret_schema = &srv_secret_schema;

    static SecretSchema chan_secret_schema = {
        .name = PACKAGE_APPID ".ChannelPassword",
        .flags = SECRET_SCHEMA_NONE,
        .attributes = {
            {
                .name = SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER,
                .type = SECRET_SCHEMA_ATTRIBUTE_STRING,
            },
            {
                .name = SRN_CONFIG_SECRET_SCHEMA_ATTR_CHANNEL,
                .type = SECRET_SCHEMA_ATTRIBUTE_STRING,
            },
            { .name = "NULL", .type = 0 },
        }
    };
    mgr->chan_secret_schema = &chan_secret_schema;

    static SecretSchema user_secret_schema = {
        .name = PACKAGE_APPID ".UserPassword",
        .flags = SECRET_SCHEMA_NONE,
        .attributes = {
            {
                .name = SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER,
                .type = SECRET_SCHEMA_ATTRIBUTE_STRING,
            },
            {
                .name = SRN_CONFIG_SECRET_SCHEMA_ATTR_USER,
                .type = SECRET_SCHEMA_ATTRIBUTE_STRING,
            },
            { .name = "NULL", .type = 0 },
        }
    };
    mgr->user_secret_schema = &user_secret_schema;
}

SecretSchema* srn_config_manager_get_server_secret_schema(SrnConfigManager *mgr) {
    return mgr->srv_secret_schema;
}

SecretSchema* srn_config_manager_get_channel_secret_schema(SrnConfigManager *mgr) {
    return mgr->chan_secret_schema;
}

SecretSchema* srn_config_manager_get_user_secret_schema(SrnConfigManager *mgr) {
    return mgr->user_secret_schema;
}

SrnRet srn_config_manager_lookup_server_password(SrnConfigManager *mgr,
        char **passwd, const char *srv_name){
    char *tmp;
    GError *err;
    SrnRet ret;

    g_return_val_if_fail(passwd, SRN_ERR);

    err = NULL;
    ret = SRN_OK;
    tmp = secret_password_lookup_sync(mgr->srv_secret_schema, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }
    if (tmp) {
        // Make sure the returnd password can be freed by g_free().
        *passwd = g_strdup(tmp);
        secret_password_free(tmp);
    }

    return ret;
}

SrnRet srn_config_manager_store_server_password(SrnConfigManager *mgr,
        const char *passwd, const char *srv_name){
    GError *err;
    SrnRet ret;

    ret = SRN_OK;
    err = NULL;
    secret_password_store_sync(mgr->srv_secret_schema,
            SECRET_COLLECTION_DEFAULT,
            _("Server password"), passwd, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }

    return ret;
}

SrnRet srn_config_manager_clear_server_password(SrnConfigManager *mgr, const char *srv_name){
    GError *err;
    SrnRet ret;

    ret = SRN_OK;
    err = NULL;
    secret_password_clear_sync(mgr->srv_secret_schema, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }

    return ret;
}

SrnRet srn_config_manager_lookup_channel_password(SrnConfigManager *mgr,
        char **passwd, const char *srv_name, const char *chan_name){
    char *tmp;
    GError *err;
    SrnRet ret;

    g_return_val_if_fail(passwd, SRN_ERR);

    err = NULL;
    ret = SRN_OK;
    tmp = secret_password_lookup_sync(mgr->chan_secret_schema, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_CHANNEL, chan_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }
    if (tmp) {
        // Make sure the returnd password can be freed by g_free().
        *passwd = g_strdup(tmp);
        secret_password_free(tmp);
    }

    return ret;
}

SrnRet srn_config_manager_store_channel_password(SrnConfigManager *mgr,
        const char *passwd, const char *srv_name, const char *chan_name){
    GError *err;
    SrnRet ret;

    ret = SRN_OK;
    err = NULL;
    secret_password_store_sync(mgr->chan_secret_schema,
            SECRET_COLLECTION_DEFAULT,
            _("Channel password"), passwd, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_CHANNEL, chan_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }

    return ret;
}

SrnRet srn_config_manager_clear_channel_password(SrnConfigManager *mgr,
        const char *srv_name, const char *chan_name){
    GError *err;
    SrnRet ret;

    ret = SRN_OK;
    err = NULL;
    secret_password_clear_sync(mgr->chan_secret_schema, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_CHANNEL, chan_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }

    return ret;
}

SrnRet srn_config_manager_lookup_user_password(SrnConfigManager *mgr,
        char **passwd, const char *srv_name, const char *user_name){
    char *tmp;
    GError *err;
    SrnRet ret;

    g_return_val_if_fail(passwd, SRN_ERR);

    err = NULL;
    ret = SRN_OK;
    tmp = secret_password_lookup_sync(mgr->user_secret_schema, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_USER, user_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }
    if (tmp) {
        // Make sure the returnd password can be freed by g_free().
        *passwd = g_strdup(tmp);
        secret_password_free(tmp);
    }

    return ret;
}

SrnRet srn_config_manager_store_user_password(SrnConfigManager *mgr,
        const char *passwd, const char *srv_name, const char *user_name){
    GError *err;
    SrnRet ret;

    ret = SRN_OK;
    err = NULL;
    secret_password_store_sync(mgr->user_secret_schema,
            SECRET_COLLECTION_DEFAULT,
            _("User password"), passwd, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_USER, user_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }

    return ret;
}

SrnRet srn_config_manager_clear_user_password(SrnConfigManager *mgr,
        const char *srv_name, const char *user_name){
    GError *err;
    SrnRet ret;

    ret = SRN_OK;
    err = NULL;
    secret_password_clear_sync(mgr->user_secret_schema, NULL, &err,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_SERVER, srv_name,
            SRN_CONFIG_SECRET_SCHEMA_ATTR_USER, user_name,
            NULL);
    if (err) {
        ret = RET_ERR("%s", err->message);
        g_error_free(err);
    }

    return ret;
}
