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

SrainChan* srain_window_add_chan(SrainWindow *win, const char *srv_name, const char *chan_name, ChatType type);
void srain_window_rm_chan(SrainWindow *win, SrainChan *chan);
SrainChan *srain_window_get_cur_chan(SrainWindow *win);
SrainChan *srain_window_get_chan_by_name(SrainWindow *win, const char *server_name, const char *chan_name);
GList* srain_window_get_chans_by_srv_name(SrainWindow *win, const char *server_name);
void srain_window_spinner_toggle(SrainWindow *win, gboolean is_busy);
void srain_window_stack_sidebar_update(SrainWindow *win, SrainChan *chan, const char *nick, const char *msg);

/* Only one SrainWindow instance in one application */
extern SrainWindow *srain_win;

#endif /* __SRAIN_WINDOW_H */
