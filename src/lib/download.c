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
 * @file download.c
 * @brief Download file from a given url and save it to cache dir
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-18
 *
 * TODO: Use libsoup instead
 */


#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <string.h>
#include "log.h"

static GString* url2path(const char *url){
    char *buf;
    char hashstr[32];
    unsigned long hash;
    GString *res;

    hash = 0;
    /* BKDR hash */
    while (*url){
        hash = hash * 131 + *url++;
    }
    snprintf(hashstr, 32, "%lu", hash);

    buf = g_build_filename(g_get_user_cache_dir(), "srain", hashstr, NULL);
    res = g_string_new(buf);
    LOG_FR("'%s'", res->str);

    g_free(buf);
    return res;
}

static int write_data(void *ptr, int size, int nmemb, FILE *stream) {
    int written;
    
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

GString *download(const char *url){
    FILE *fp;
    GString *path;
    CURL *curl;
    CURLcode res = 0;

    LOG_FR("%s", url);
    path = url2path(url);
    if (access(path->str, F_OK ) != -1){
        LOG_FR("exist, return");
        return path;
    }

    fp = fopen(path->str, "wb");
    if (!fp){
        ERR_FR("failed to creating '%s'", path->str);
        return NULL;
    }

    curl = curl_easy_init();
    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    fclose(fp);

    if (res != CURLE_OK){
        unlink(path->str);
        ERR_FR("error code %d", res);
        return NULL;
    }

    LOG_FR("saved");
    return path;
}
