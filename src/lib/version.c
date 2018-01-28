/* Copyright (C) 2016-2018 Shengyu Zhang <srain@srain.im>
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
 * @file version.c
 * @brief Semantic version parser
 * @author Shengyu Zhang <i@srain.im>
 * @version 0.06.3
 * @date 2018-01-28
 *
 * Format: MAJOR.MINOR.MICRO-BUILD, for example: 0.7.0-git@deadbeef
 *
 * Ref: https://github.com/semver/semver/blob/master/semver.md
 */

#include <glib.h>
#include <string.h>

#include "version.h"
#include "utils.h"
#include "i18n.h"

SrnVersion *srn_version_new(const char *raw){
    SrnVersion *ver;

    ver = g_malloc0(sizeof(SrnVersion));
    str_assign(&ver->raw, raw);
    
    return ver;
}

void srn_version_free(SrnVersion *ver){
    str_assign(&ver->raw, NULL);
    str_assign(&ver->build, NULL);
    g_free(ver);
}

SrnRet srn_version_parse(SrnVersion *ver){
    unsigned int num;
    char *raw;
    char *build;
    char *endptr;
    char **strv;
    SrnRet ret;

    ret = RET_ERR("Invalid version string");
    raw = g_strdup(ver->raw);
    build = NULL;
    strv = NULL;

    build = strchr(raw, '-');
    if (build){
        *build++ = '\0';
        str_assign(&ver->build, build);
    }

    strv = g_strsplit(raw, ".", 0);
    switch (g_strv_length(strv)){
        case 3:
            num = g_ascii_strtoull(strv[2], &endptr, 10);
            if (endptr == strv[2]){
                goto FIN;
            }
            ver->micro = num;
            // Fallthrough
        case 2:
            num = g_ascii_strtoull(strv[1], &endptr, 10);
            if (endptr == strv[1]){
                goto FIN;
            }
            ver->minor = num;
            // Fallthrough
        case 1:
            num = g_ascii_strtoull(strv[0], &endptr, 10);
            if (endptr == strv[0]){
                goto FIN;
            }
            ver->major = num;
            break;
        case 0:
            goto FIN;
        default:
            goto FIN;
    }

    ret = SRN_OK;

FIN:
    if (raw) {
        g_free(raw);
    }
    if (strv) {
        g_strfreev(strv);
    }

    return ret;
}
