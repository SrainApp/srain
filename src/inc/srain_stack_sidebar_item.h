#ifndef __SRAIN_STACK_SIDEBAR_ITEM_H
#define __SRAIN_STACK_SIDEBAR_ITEM_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_STACK_SIDEBAR_ITEM (srain_stack_sidebar_item_get_type())
#define SRAIN_STACK_SIDEBAR_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_STACK_SIDEBAR_ITEM, SrainStackSidebarItem))
#define SRAIN_IS_STACK_SIDEBAR_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_STACK_SIDEBAR_ITEM))

typedef struct _SrainStackSidebarItem SrainStackSidebarItem;
typedef struct _SrainStackSidebarItemClass SrainStackSidebarItemClass;

GType srain_stack_sidebar_item_get_type(void);
SrainStackSidebarItem *srain_stack_sidebar_item_new(const char *server_name, const char *chan_name);
void srain_stack_sidebar_item_count_clear(SrainStackSidebarItem *item);
void srain_stack_sidebar_item_count_inc(SrainStackSidebarItem *item);
void srain_stack_sidebar_item_recentmsg_update(SrainStackSidebarItem *item, const char *nick, const char *msg);

#endif /* __SRAIN_STACK_SIDEBAR_ITEM_H */

