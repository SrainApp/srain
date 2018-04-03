/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LOG_H
#define __LOG_H

#include <glib.h>

#include "srain.h"
#include "ret.h"

typedef enum _SrnLogLevel SrnLogLevel;
typedef struct _SrnLoggerConfig SrnLoggerConfig;
typedef struct _SrnLogger SrnLogger;

enum _SrnLogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_MAX,
};

struct _SrnLoggerConfig {
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
    srn_logger_log(srn_logger_get_default(), LOG_DEBUG, TRUE, TRUE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define DBG_F(...) \
    srn_logger_log(srn_logger_get_default(), LOG_DEBUG, TRUE, FALSE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define DBG(...) \
    srn_logger_log(srn_logger_get_default(), LOG_DEBUG, FALSE, FALSE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/* Info output */
#define LOG_FR(...) \
    srn_logger_log(srn_logger_get_default(), LOG_INFO, TRUE, TRUE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG_F(...) \
    srn_logger_log(srn_logger_get_default(), LOG_INFO, TRUE, FALSE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define LOG(...) \
    srn_logger_log(srn_logger_get_default(), LOG_INFO, FALSE, FALSE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/* Warn output */
#define WARN_FR(...) \
    srn_logger_log(srn_logger_get_default(), LOG_WARN, TRUE, TRUE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define WARN_F(...) \
    srn_logger_log(srn_logger_get_default(), LOG_WARN, TRUE, FALSE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define WARN(...) \
    srn_logger_log(srn_logger_get_default(), LOG_WARN, FALSE, FALSE \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define ERR_FR(...) \
    srn_logger_log(srn_logger_get_default(), LOG_ERROR, TRUE, TRUE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define ERR_F(...) \
    srn_logger_log(srn_logger_get_default(), LOG_ERROR, TRUE, FALSE, \
            __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

SrnLogger *srn_logger_get_default(void);
void srn_logger_set_default(SrnLogger *logger);

SrnLogger *srn_logger_new(SrnLoggerConfig *cfg);
void srn_logger_free(SrnLogger *logger);
void srn_logger_set_config(SrnLogger *logger, SrnLoggerConfig *cfg);
SrnLoggerConfig *srn_logger_get_config(SrnLogger *logger);
void srn_logger_log(SrnLogger *logger, SrnLogLevel lv, bool print_prompt,
        bool new_line, const char *file, const char *func, int line,
        const char *fmt, ...);

SrnLoggerConfig *srn_logger_config_new(void);
void srn_logger_config_free(SrnLoggerConfig *cfg);
SrnRet srn_logger_config_check(SrnLoggerConfig *cfg);

#endif /* __LOG_H */
