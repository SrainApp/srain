#include <strings.h>
#include <glib.h>

#include "srain.h"

bool sirc_nick_cmp(const char *nick1, const char *nick2){
    return strcasecmp(nick1, nick2) == 0;
}

bool sirc_prefix_is_server(const char *prefix){
    return NULL;
}

/* TODO */
const char* sirc_prefix_get_nick(const char *prefix){
    return NULL;
}

const char* sirc_prefix_get_host(const char *prefix){
    return NULL;
}

const char* sirc_prefix_get_user(const char *prefix){
    return NULL;
}
