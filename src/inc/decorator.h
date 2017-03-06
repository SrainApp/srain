#ifndef __DECORATOR_H
#define __DECORATOR_H

#include "server.h"

#define DECORATOR_MIRC_COLORIEZ 1 << 0
#define DECORATOR_MIRC_STRIP    1 << 1

#define DECORATOR_NICK_COLORIZE 1 << 2
#define DECORATOR_BOT2HUMAN     1 << 3
#define DECORATOR_NICK_HIGHLIGH 1 << 4
#define DECORATOR_CHAT_LOG      1 << 5

typedef int DecoratorFlag;

typedef int (DecoratorFunc) (Message *msg, DecoratorFlag flag, void *user_data);;

typedef struct _Decorator {
    const char *name;
    DecoratorFunc *func;
} Decorator;

int decorate_message(Message *msg, DecoratorFlag flag, void *user_data);

#endif /* __DECORATOR_H */
