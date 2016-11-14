#ifndef __FILE_HELPER_H
#define __FILE_HELPER_H

char *get_theme_file(const char *fname);
char *get_pixmap_file(const char *fname);
char *get_plugin_file(const char *fname);
char *get_config_file(const char *fname);
char *get_avatar_file(const char *fname);
char *create_log_file(const char *srv_name, const char *fname);
int create_user_file();

#endif /* __FILE_HELPER_H */
