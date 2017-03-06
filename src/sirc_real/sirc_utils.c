#include <strings.h>

#include "srain.h"

bool sirc_nick_cmp(const char *nick1, const char *nick2){
    return strcasecmp(nick1, nick2) == 0;
}

bool sirc_prefix_is_server(const char *prefix){
}

const char* sirc_prefix_get_nick(const char *prefix){

}

const char* sirc_prefix_get_host(const char *prefix){

}

const char* sirc_prefix_get_user(const char *prefix){

}
