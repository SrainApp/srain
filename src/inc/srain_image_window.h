#ifndef __SRAIN_IMAGE_WINDOW_H
#define __SRAIN_IMAGE_WINDOW_H

#include <gtk/gtk.h>

#define SRAIN_TYPE_IMAGE_WINDOW (srain_image_window_get_type())
#define SRAIN_IMAGE_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_IMAGE_WINDOW, SrainImageWindow))

typedef struct _SrainImageWindow SrainImageWindow;
typedef struct _SrainImageWindowClass SrainImageWindowClass;

GType srain_image_window_get_type(void);
SrainImageWindow *srain_image_window_new(const char *path);

#endif /* __SRAIN_IMAGE_WINDOW_H */
