#ifndef __SRAIN_WINDOW_H
#define __SRAIN_WINDOW_H

#include <gtk/gtk.h>
#include "srain_app.h"


#define SRAIN_TYPE_WINDOW (srain_window_get_type())
#define SRAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_WINDOW, SrainWindow))

typedef struct _SrainWindow SrainWindow;
typedef struct _SrainWindowClass SrainWindowClass;

GType srain_window_get_type(void);
SrainWindow *srain_window_new(SrainApp *app);

#endif /* __EXAMPLEAPPWIN_H */
