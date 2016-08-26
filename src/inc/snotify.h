#ifndef __SNOTIFY_H
#define __SNOTIFY_H

void snotify_init();
void snotify_notify(const char *title, const char *msg, const char *icon);
void snotify_finalize();

#endif /* __SNOTIFY_H */
