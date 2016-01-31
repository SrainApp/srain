#ifndef __MSG_H
#define __MSG_H

typedef struct {
    char *id;
    char *nick;
    char *chan;
    char *msg;
    char *time;
    char *avatar;   // path of cached avatar, can be null
    char *img;      // path of cached img, can be null
} MsgRecv;

typedef struct {
    char *nick;
    char *chan;
    char *msg;
    void *time;
    char *img;      // path of cached img, can be null
} MsgSend;

typedef struct {
    char *chan;
    char *msg;
} MsgSys;

#endif /* __MSG_H */
