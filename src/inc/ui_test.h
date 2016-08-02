#ifndef __UI_TEST_H
#define __UI_TEST_H

int ui_test_srv_join(const char *srv_name, const char *chan_name, const char *passwd);
int ui_test_srv_part(const char *srv_name, const char *chan_name, const char *reason);
int ui_test_srv_send(const char *srv_name, const char *chan_name, const char *msg);
int ui_test_srv_cmd(const char *srv_name, const char *chan_name, char *cmd, int block);
void ui_test();

#endif /* __UI_TEST_H */
