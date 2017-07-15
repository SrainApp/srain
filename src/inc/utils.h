#ifndef __UTILS_H
#define __UTILS_H

#include <time.h>

unsigned long get_time_since_first_call_ms(void);
time_t get_current_time_s(void);
void time_to_str(time_t time, char *timestr, size_t size, const char *fmt);
void str_assign(char **left, const char *right);

#endif /* __UTILS_H */
