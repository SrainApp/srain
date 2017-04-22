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
 * NOTE: This function is not thread-safed
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
