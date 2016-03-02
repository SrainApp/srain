#ifndef __LOG_H
#define __LOG_H

#include <stdlib.h>
#include <glib.h>

// log macro with Line number, Fucntion name and Return
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

#define _LOG_LFR(...)                                \
    g_print("%d: [%s]: ", __LINE__, __FUNCTION__);  \
    g_print(__VA_ARGS__);                           \
    g_print("\n")

// log macro with Fucntion name and Return
#define _LOG_FR(...)                     \
    g_print("[%s]: ",  __FUNCTION__);   \
    g_print(__VA_ARGS__);               \
    g_print("\n")

// log macro with Fucntion name
#define _LOG_F(...)                     \
    g_print("[%s]: ",  __FUNCTION__);   \
    g_print(__VA_ARGS__);

// log macro, equal to g_print
#define _LOG(...)            \
    g_print(__VA_ARGS__);   \

// printerr macro with Line number, Fucntion name and Return
#define ERR_LFR(...)                                            \
    g_print("%d: **ERROR** [%s]: ", __LINE__, __FUNCTION__);    \
    g_print(__VA_ARGS__);                                       \
    g_print("\n")

// printerr macro with Fucntion name and Return
#define ERR_FR(...)                                 \
    g_printerr("**ERROR** [%s]: ",  __FUNCTION__);  \
    g_printerr(__VA_ARGS__);                        \
    g_printerr("\n")

// printerr macro, equal to g_printerr
#define ERR(...)                \
    g_printerr("**ERROR** ");   \
    g_printerr(__VA_ARGS__);    \

#endif /* __LOG_H */
