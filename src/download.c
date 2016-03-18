/**
 * @file download.c
 * @brief provide funciton download(), download file(oftenly image file)
 *      from a given url
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-18
 */

#define __LOG_ON

#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include "log.h"

static GString* gen_path(const char *ext, int len){
    int idx;
    static int is_init = 0;
    GString *path;
    char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (!is_init){
        srand(time(NULL));
        is_init = 1;
    }

    path = g_string_new("./");

    while (len--) {
        idx = rand()*1.0/RAND_MAX*(sizeof(charset) - 1);
        path = g_string_append_c(path, charset[idx]);
    }

    path = g_string_append(path, ext);
    LOG_FR("'%s'",path->str);

    return path;
}

static int write_data(void *ptr, int size, int nmemb, FILE *stream) {
    int written;
    
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

GString *download(const char *url){
    char *ext;
    FILE *fp;
    CURL *curl;
    CURLcode res;
    GString *path;

    if ((ext = strrchr(url, '.')) == NULL){
        /* oftenly it is impossible */
        ERR_FR("file extend name not found");
        return NULL;
    }
    path = gen_path(ext, 32);

    curl = curl_easy_init();
    if (curl){
        fp = fopen(path->str, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);

        fclose(fp);
    }

    if (res != CURLE_OK){
        ERR_FR("error code %d", res);
        return NULL;
    }

    return path;
}
