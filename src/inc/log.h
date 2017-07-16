#ifndef __LOG_H
#define __LOG_H

#include <stdlib.h>
#include <glib.h>

#include "srain.h"
#include "ret.h"

typedef enum _LogLevel LogLevel;
typedef struct _LogPrefs LogPrefs;

enum _LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_MAX,
};

struct _LogPrefs {
    bool prompt_color;
    bool prompt_file;
    bool prompt_function;
    bool prompt_line;

    GSList *debug_targets;
    GSList *info_targets;
    GSList *warn_targets;
    GSList *error_targets;
};
/* Debug output */
#define DBG_FR(...) \
    log_print(LOG_DEBUG, TRUE, TRUE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define DBG_F(...) \
    log_print(LOG_DEBUG, TRUE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define DBG(...) \
    log_print(LOG_DEBUG, FALSE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/* Info output */
#define LOG_FR(...) \
    log_print(LOG_INFO, TRUE, TRUE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG_F(...) \
    log_print(LOG_INFO, TRUE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG(...) \
    log_print(LOG_INFO, FALSE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/* Warn output */
#define WARN_FR(...) \
    log_print(LOG_WARN, TRUE, TRUE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define WARN_F(...) \
    log_print(LOG_WARN, TRUE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define WARN(...) \
    log_print(LOG_WARN, FALSE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define ERR_FR(...) \
    log_print(LOG_ERROR, TRUE, TRUE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define ERR_F(...) \
    log_print(LOG_ERROR, TRUE, FALSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

void log_init();
SrnRet log_read_prefs();
void log_finalize();
void log_print(LogLevel lv, bool print_prompt, bool new_line,
        const char *file, const char *func, int line,
        const char *fmt, ...);

#endif /* __LOG_H */
