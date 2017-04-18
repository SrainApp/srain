#ifndef __SRAIN_MSG_LIST_H
#define __SRAIN_MSG_LIST_H

#include <gtk/gtk.h>
#include "srain_msg.h"

#define SRAIN_TYPE_MSG_LIST (srain_msg_list_get_type())
#define SRAIN_MSG_LIST(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_MSG_LIST, SrainMsgList))
#define SRAIN_IS_MSG_LIST(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_MSG_LIST))

typedef struct _SrainMsgList SrainMsgList;
typedef struct _SrainMsgListClass SrainMsgListClass;

GType srain_msg_list_get_type(void);
SrainMsgList *srain_msg_list_new(void);

void srain_msg_list_add_message(SrainMsgList *list, SuiMessage *smsg);
void srain_msg_list_highlight_message(SuiMessage *smsg);

void srain_msg_list_scroll_up(SrainMsgList *list, double step);
void srain_msg_list_scroll_down(SrainMsgList *list, double step);

#endif /* __SRAIN_MSG_LIST_H */
