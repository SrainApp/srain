#ifndef __SIRC_UTILS_H
#define  __SIRC_UTILS_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include "srain.h"

#define CHAN_PREFIX1    '#'
#define CHAN_PREFIX2    '&'

#define sirc_is_chan(ch) (ch && (ch[0] == CHAN_PREFIX1 || ch[0] == CHAN_PREFIX2))

bool sirc_nick_cmp(const char *nick1, const char *nick2);
bool sirc_prefix_is_server(const char *prefix);

const char* sirc_prefix_get_nick(const char *prefix);
const char* sirc_prefix_get_host(const char *prefix);
const char* sirc_prefix_get_user(const char *prefix);

#endif /* __SIRC_UTILS_H */
