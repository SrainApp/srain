#ifndef __LOG_H
#define __LOG_H

#include <stdlib.h>
#include <glib.h>

#ifdef __DBG_ON
    #define DBG_LFR(...) _DBG_LFR(__VA_ARGS__)
    #define DBG_FR(...) _DBG_FR(__VA_ARGS__)
    #define DBG_F(...) _DBG_F(__VA_ARGS__)
    #define DBG(...) _DBG(__VA_ARGS__)
#else
    #define DBG_LFR(...) ;
    #define DBG_FR(...) ;
    #define DBG_F(...) ;
    #define DBG(...) ;
#endif

/* Print a line of debug message with fucntion name */
#define _DBG_FR(...)                            \
    do {                                        \
        g_print("[DBG][%s]: ",  __FUNCTION__); \
        g_print(__VA_ARGS__);                   \
        g_print("\n");                          \
    } while (0);

/* Print a line of debug message with fucntion name, 
 * do not output the trailing newline */
#define _DBG_F(...)                         \
    do {                                    \
        g_print("[DBG][%s]: ",  __FUNCTION__); \
        g_print(__VA_ARGS__);               \
    } while (0);

/* Print a log */
#define _DBG(...)               \
    do {                        \
        g_print(__VA_ARGS__);   \
    } while (0);

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
#define _LOG_FR(...)                                \
    do {                                            \
        g_print("[INFO][%s]: ",  __FUNCTION__);    \
        g_print(__VA_ARGS__);                       \
        g_print("\n");                              \
    } while (0);

/* Print a line of log with fucntion name
 * do not output the trailing newline */
#define _LOG_F(...)                                 \
    do {                                            \
        g_print("[INFO][%s]: ",  __FUNCTION__);    \
        g_print(__VA_ARGS__);                       \
    } while (0);

/* Print a log */
#define _LOG(...)               \
    do {                        \
        g_print(__VA_ARGS__);   \
    } while (0);

/* Print a line of warning message with fucntion name */
#define WARN_FR(...)                                    \
    do {                                                \
        g_printerr("**WARN** [%s]: ",  __FUNCTION__);   \
        g_printerr(__VA_ARGS__);                        \
        g_printerr("\n");                               \
    } while (0);

/* Print a line of error message with fucntion name */
#define ERR_FR(...)                                 \
    do {                                            \
        g_printerr("**ERROR** [%s:%d][%s]: ",       \
                __FILE__, __LINE__, __FUNCTION__);  \
        g_printerr(__VA_ARGS__);                    \
        g_printerr("\n");                           \
    } while (0);

#endif /* __LOG_H */
