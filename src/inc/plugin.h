#ifndef __PLUGIN_H
#define __PLUGIN_H

int plugin_init();
char* plugin_upload(const char *path);
char* plugin_avatar(const char *nick, const char *user, const char *host);

#endif /* __PLUGIN_H */

