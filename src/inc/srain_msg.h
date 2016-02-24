#ifndef __SRAIN_MSG_H
#define __SRAIN_MSG_H

#include <gtk/gtk.h>

/* SRAIN_SYS_MSG */
#define SRAIN_TYPE_SYS_MSG (srain_sys_msg_get_type())
#define SRAIN_SYS_MSG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_SYS_MSG, SrainSysMsg))
#define SRAIN_IS_SYS_MSG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_SYS_MSG))

typedef struct _SrainSysMsg SrainSysMsg;
typedef struct _SrainSysMsgClass SrainSysMsgClass;

GType srain_sys_msg_get_type(void);
SrainSysMsg *srain_sys_msg_new(const char *msg);

/* SRAIN_SEND_MSG */
#define SRAIN_TYPE_SEND_MSG (srain_send_msg_get_type())
#define SRAIN_SEND_MSG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_SEND_MSG, SrainSendMsg))
#define SRAIN_IS_SEND_MSG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_SEND_MSG))

typedef struct _SrainSendMsg SrainSendMsg;
typedef struct _SrainSendMsgClass SrainSendMsgClass;

GType srain_send_msg_get_type(void);
SrainSendMsg *srain_send_msg_new(const char *msg, const char *img_path);

/* SRAIN_SYS_RECV_MSG */
#define SRAIN_TYPE_RECV_MSG (srain_recv_msg_get_type())
#define SRAIN_RECV_MSG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_RECV_MSG, SrainRecvMsg))
#define SRAIN_IS_RECV_MSG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_RECV_MSG))

typedef struct _SrainRecvMsg SrainRecvMsg;
typedef struct _SrainRecvMsgClass SrainRecvMsgClass;

GType srain_recv_msg_get_type(void);
SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg, const char *img_path);

#endif /* __SRAIN_MSG_H */
