#ifndef __LOG_H
#define __LOG_H

#include <stdlib.h>
#include <glib.h>

#ifdef __LOG_ON
    #define LOG_LFR(...) _LOG_LFR(__VA_ARGS__)
    #define LOG_FR(...) _LOG_FR(__VA_ARGS__)
    #define LOG_F(...) _LOG_F(__VA_ARGS__)
    #define LOG(...) _LOG(__VA_ARGS__)
#else
    #define LOG_LFR(...) ;
    #define LOG_FR(...) ;
    #define LOG_F(...) ;
    #define LOG(...) ;
#endif

/* Print a line of log with fucntion name */
#define _LOG_FR(...)                        \
    do {                                    \
        g_print("[%s]: ",  __FUNCTION__);   \
        g_print(__VA_ARGS__);               \
        g_print("\n");                      \
    } while (0);

/* Print a line of log with fucntion name
 * do not output the trailing newline */
#define _LOG_F(...)                         \
    do {                                    \
        g_print("[%s]: ",  __FUNCTION__);   \
        g_print(__VA_ARGS__);               \
    } while (0);

/* Print a log */
#define _LOG(...)               \
    do {                        \
        g_print(__VA_ARGS__);   \
    } while (0);

/* Print a line of error message with fucntion name */
#define ERR_FR(...)                                     \
    do {                                                \
        g_printerr("**ERROR** [%s]: ",  __FUNCTION__);  \
        g_printerr(__VA_ARGS__);                        \
        g_printerr("\n");                               \
    } while (0);

/* Print a error message */
#define ERR(...)                    \
    do {                            \
        g_printerr("**ERROR** ");   \
        g_printerr(__VA_ARGS__);    \
    } while (0);

#endif /* __LOG_H */
