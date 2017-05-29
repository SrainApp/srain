#ifndef __PREFS_H
#define __PREFS_H

#include "srain.h"
#include "server.h"

void prefs_init();
const char* prefs_read();
void prefs_finalize();

#endif /*__PREFS_H */
