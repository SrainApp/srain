#ifndef __SRAIN_ABOUT_BOX_H
#define __SRAIN_ABOUT_BOX_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_ABOUT_BOX (srain_about_box_get_type())
#define SRAIN_ABOUT_BOX(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_ABOUT_BOX, SrainAboutBox))

typedef struct _SrainAboutBox SrainAboutBox;
typedef struct _SrainAboutBoxClass SrainAboutBoxClass;

GType srain_about_box_get_type(void);
SrainAboutBox *srain_about_box_new(void);

#endif /* __SRAIN_ABOUT_BOX_H */
