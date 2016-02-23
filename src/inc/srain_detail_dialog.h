#ifndef __SRAIN_DETAIL_DIALOG_H
#define __SRAIN_DETAIL_DIALOG_H

#include <gtk/gtk.h>
#include <srain_window.h>

#define SRAIN_TYPE_DETAIL_DIALOG (srain_detail_dialog_get_type())
#define SRAIN_DETAIL_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_DETAIL_DIALOG, SrainDetailDialog))

typedef struct _SrainDetailDialog SrainDetailDialog;
typedef struct _SrainDetailDialogClass SrainDetailDialogClass;

GType srain_detail_dialog_get_type(void);
SrainDetailDialog* srain_detail_dialog_new(SrainWindow *win, const char *name, const char *content);

#endif /* __SRAIN_DETAIL_DIALOG_H */
