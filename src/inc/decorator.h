#ifndef __DECORATOR_H
#define __DECORATOR_H

#include "server.h"

#define DECORATOR_BOT2HUMAN     1 << 0
#define DECORATOR_MIRC_COLORIEZ 1 << 1
#define DECORATOR_MIRC_STRIP    1 << 2
#define DECORATOR_PANGO_MARKUP  1 << 3

typedef int DecoratorFlag;

typedef int (DecoratorFunc) (Message *msg, DecoratorFlag flag, void *user_data);;

typedef struct _Decorator {
    const char *name;
    DecoratorFunc *func;
} Decorator;

void decorator_init();
int decorate_message(Message *msg, DecoratorFlag flag, void *user_data);

#endif /* __DECORATOR_H */
