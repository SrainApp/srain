/**
 * @file download.c
 * @brief download file from a given url and save it to cache dir
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-18
 *
 */

// #define __LOG_ON

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
    CURLcode res;

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
