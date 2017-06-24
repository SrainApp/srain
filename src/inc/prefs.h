#ifndef __PREFS_H
#define __PREFS_H

#include "srain.h"
#include "server.h"
#include "log.h"

void prefs_init();
char* prefs_read();
void prefs_finalize();

char* prefs_read_log_prefs(LogPrefs *prefs);
char* prefs_read_sui_app_prefs(SuiAppPrefs *prefs);
char* prefs_read_server_prefs_list();
char* prefs_read_server_prefs(ServerPrefs *prefs);
char* prefs_read_sirc_prefs(SircPrefs *prefs, const char *srv_name);
char* prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name, const char *chat_name);

#endif /*__PREFS_H */
