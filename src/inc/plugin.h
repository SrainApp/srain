#ifndef __PLUGIN_H
#define __PLUGIN_H

#include <glib.h>

void plugin_init();
void plugin_finalize();
char* plugin_upload(const char *path);
int plugin_avatar(const char *nick, const char *token);

#endif /* __PLUGIN_H */

