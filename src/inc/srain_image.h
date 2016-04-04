#ifndef __SRAIN_IMAGE_H
#define __SRAIN_IMAGE_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_IMAGE (srain_image_get_type())
#define SRAIN_IMAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_IMAGE, SrainImage))

typedef struct _SrainImage SrainImage;
typedef struct _SrainImageClass SrainImageClass;

GType srain_image_get_type(void);
SrainImage *srain_image_new(void);
SrainImage* srain_image_new_from_url_async(const char *url, int size, int if_autoload);

#endif /* __SRAIN_IMAGE_H */
