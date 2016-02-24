#ifndef __SRAIN_WINDOW_H
#define __SRAIN_WINDOW_H

#include <gtk/gtk.h>
#include "srain_app.h"
#include "srain_chan.h"


#define SRAIN_TYPE_WINDOW (srain_window_get_type())
#define SRAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SRAIN_TYPE_WINDOW, SrainWindow))
#define SRAIN_IS_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SRAIN_TYPE_WINDOW))

typedef struct _SrainWindow SrainWindow;
typedef struct _SrainWindowClass SrainWindowClass;

GType srain_window_get_type(void);
SrainWindow *srain_window_new(SrainApp *app);

SrainChan* srain_window_add_chan(SrainWindow *win, const char *name);
int srain_window_rm_chan(SrainWindow *win, const char *name);
SrainChan *srain_window_get_cur_chan(SrainWindow *win);
SrainChan *srain_window_get_chan_by_name(SrainWindow *win, const char *name);
void srain_window_spinner_toggle(SrainWindow *win, gboolean is_busy);

#endif /* __SRAIN_WINDOW_H */
