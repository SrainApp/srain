#ifndef __SIRC_LOADER_H
#define __SIRC_LOADER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SPY_TYPE_LOADER spy_loader_get_type()
G_DECLARE_FINAL_TYPE(SpyLoader, spy_loader, SPY, LOADER, GObject)

G_END_DECLS

#endif /* __SIRC_LOADER_H */
