/**
 * @file utils.c
 * @brief Utility functions
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-04-22
 */

#include <glib.h>

/* Datetime utilities */

/**
 * @brief Get the time since this function firstly called, in millsceonds
 *
 * @return 
 *
 * NOTE: This function is not thread-safe
 **/
unsigned long get_time_since_first_call_ms(void){
    static gint64 first = 0;
    unsigned long ret;

    if (first == 0){
        first = g_get_monotonic_time() / 1000;
        ret = 0;
    } else {
        ret = (g_get_monotonic_time() / 1000) - first;
    }

    return ret;
}

time_t get_current_time_s(void){
    GTimeVal val;

    g_get_current_time(&val);

    return val.tv_sec + val.tv_usec / 1e6;
}

void time_to_str(time_t time, char *timestr, size_t size, const char *fmt){
    strftime(timestr, size - 1, fmt, localtime(&time));

    timestr[size-1] = '\0';
}

void str_assign(char **left, const char *right){
    if (*left) {
        g_free(*left);
    }
    *left = g_strdup(right);
}
