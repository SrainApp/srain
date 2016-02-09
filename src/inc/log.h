#ifndef __LOG_H
#define __LOG_H

#include <glib.h>

// log macro with Line number, Fucntion name and Return
#define LOG_LFR(...)                                \
    g_print("%d: [%s]: ", __LINE__, __FUNCTION__);  \
    g_print(__VA_ARGS__);                           \
    g_print("\n")

// log macro with Fucntion name and Return
#define LOG_FR(...)                     \
    g_print("[%s]: ",  __FUNCTION__);   \
    g_print(__VA_ARGS__);               \
    g_print("\n")

// log macro with Fucntion name
#define LOG_F(...)                     \
    g_print("[%s]: ",  __FUNCTION__);   \
    g_print(__VA_ARGS__);

// log macro, equal to g_print
#define LOG(...)            \
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
