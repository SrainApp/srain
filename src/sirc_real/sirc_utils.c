#include <strings.h>
#include <glib.h>

#include "srain.h"

bool sirc_nick_cmp(const char *nick1, const char *nick2){
    g_return_val_if_fail(nick1, FALSE);
    g_return_val_if_fail(nick2, FALSE);

    return g_ascii_strcasecmp(nick1, nick2) == 0;
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
