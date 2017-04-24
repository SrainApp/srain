#ifndef __SRAIN_MSG_H
#define __SRAIN_MSG_H

#include <gtk/gtk.h>

#include "sui/sui.h"
#include "sui_message.h"

#define SRAIN_MSG_MAX_LEN 512

/* ================ SRAIN_SRAIN_MSG ================ */

typedef struct _SuiMessage SrainMsg;

#define SRAIN_MSG(obj) ((SrainMsg *) obj)
#define SRAIN_IS_MSG(obj) \
    (SRAIN_IS_SYS_MSG(obj) || \
     SRAIN_IS_SEND_MSG(obj) || \
     SRAIN_IS_RECV_MSG(obj))

int srain_msg_append_msg(SrainMsg *smsg, const char *msg);
void srain_msg_append_image(SrainMsg *smsg, const char *url);

/* ================ SRAIN_SYS_MSG ================ */

struct _SrainSysMsg {
    SUI_MESSAGE;

    SysMsgType type;
};

struct _SrainSysMsgClass {
    GtkBoxClass parent_class;
};

#define SRAIN_TYPE_SYS_MSG (srain_sys_msg_get_type())
#define SRAIN_SYS_MSG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_SYS_MSG, SrainSysMsg))
#define SRAIN_IS_SYS_MSG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_SYS_MSG))

typedef struct _SrainSysMsg SrainSysMsg;
typedef struct _SrainSysMsgClass SrainSysMsgClass;

GType srain_sys_msg_get_type(void);
SrainSysMsg* srain_sys_msg_new(const char *msg, SysMsgType type);

/* ================ SRAIN_SEND_MSG ================ */
struct _SrainSendMsg {
    SUI_MESSAGE;

    GString *image_path;
};

struct _SrainSendMsgClass {
    GtkBoxClass parent_class;
};

#define SRAIN_TYPE_SEND_MSG (srain_send_msg_get_type())
#define SRAIN_SEND_MSG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_SEND_MSG, SrainSendMsg))
#define SRAIN_IS_SEND_MSG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_SEND_MSG))

typedef struct _SrainSendMsg SrainSendMsg;
typedef struct _SrainSendMsgClass SrainSendMsgClass;

GType srain_send_msg_get_type(void);
SrainSendMsg *srain_send_msg_new(const char *msg);

/* ================ SRAIN_RECV_MSG ================ */
struct _SrainRecvMsg {
    SUI_MESSAGE;

    GtkImage *avatar_image;
    GString *image_path;
    GtkLabel *nick_label;
    GtkLabel *identify_label;
    GtkButton *nick_button;
};

struct _SrainRecvMsgClass {
    GtkBoxClass parent_class;
};

#define SRAIN_TYPE_RECV_MSG (srain_recv_msg_get_type())
#define SRAIN_RECV_MSG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_RECV_MSG, SrainRecvMsg))
#define SRAIN_IS_RECV_MSG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_RECV_MSG))

typedef struct _SrainRecvMsg SrainRecvMsg;
typedef struct _SrainRecvMsgClass SrainRecvMsgClass;

GType srain_recv_msg_get_type(void);
SrainRecvMsg *srain_recv_msg_new(const char *nick, const char *id, const char *msg);

#endif /* __SRAIN_MSG_H */
