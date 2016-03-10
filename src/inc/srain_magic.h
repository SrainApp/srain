#ifndef __SRAIN_MAGIC_H
#define __SRAIN_MAGIC_H

#define SERVER_LEN      64      // unconfirm
#define HOST_LEN        64
#define CHAN_LEN        200
#define NICK_LEN        128     // unconfirm
#define USER_LEN        128     // unconfirm
#define COMMAND_LEN     64      // unconfirm
#define MSG_LEN         512
#define PARAM_COUNT     64      // unconfirm
#define PARAM_LEN       64      // unconfirm
#define CHAN_PREFIX1    '#'
#define CHAN_PREFIX2    '&'
// It seems that "Server" is a illegal nick in Freenode,
// so use it as name of the server you connected to
#define SERVER          "Server"

#endif /* __SRAIN_MAGIC_H */
