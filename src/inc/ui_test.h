
#ifndef __UI_TEST_H
#define __UI_TEST_H

void ui_test_server_name_join(const char *server_name, const char *chan_name);
void ui_test_server_name_part(const char *server_name, const char *chan_name);
void ui_test_server_name_send(const char *server_name, const char *chan_name, const char *msg);
void ui_test_server_name_cmd(const char *server_name, const char *chan_name, const char *cmd);
void ui_test();

#endif /* __UI_TEST_H */
