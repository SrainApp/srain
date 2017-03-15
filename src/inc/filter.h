#ifndef __FILTER_H
#define __FILTER_H

#include "srain.h"
#include "server.h"

#define FILTER_NICK     1 << 0
#define FILTER_REGEX    1 << 1

typedef int FilterFlag;

typedef int (FilterFunc) (const Message *msg, FilterFlag flag, void *user_data);;

typedef struct _Filter {
    const char *name;
    FilterFunc *func;
} Filter;

void filter_init();
bool filter_message(const Message *msg, FilterFlag flag, void *user_data);

#endif /* __FILTER_H */
