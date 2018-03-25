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

/**
 * @file log.c
 * @brief A configurable logger
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2017-06-24
 */

#include <stdio.h>
#include <glib.h>

#include "srain.h"
#include "config/config.h"
#include "log.h"

struct _SrnLogger {
    SrnLoggerConfig *cfg;
};

static SrnLogger *srn_logger = NULL;

static char *prompts[2][LOG_MAX] = {
    [0] = {
        [LOG_DEBUG] = "[ DBG %s] ",
        [LOG_INFO]  = "[INFO %s] ",
        [LOG_WARN]  = "[WARN %s] ",
        [LOG_ERROR] = "[ ERR %s] ",
    },
    [1] = {
        [LOG_DEBUG] = "\e[37m[ DBG %s]\e[0m ",
        [LOG_INFO]  = "\e[32m[INFO %s]\e[0m ",
        [LOG_WARN]  = "\e[33m[WARN %s]\e[0m ",
        [LOG_ERROR] = "\e[31m[ ERR %s]\e[0m ",
    },
};

static bool is_exist(GSList *files, const char *file);
static bool is_enabled(SrnLoggerConfig *cfg, SrnLogLevel lv, const char *file);

/**
 * @brief log_print Print a log
 *
 * @param logger Instance of SrnLogger
 * @param lv Log level
 * @param print_prompt
 * @param new_line
 * @param file File name, shoule be the value of __FILE__
 * @param func Function name, shoule be the value of __FUNCTION__
 * @param line Line number in file, should be the value of __LINE__
 * @param fmt Format string
 * @param ...
 */
SrnLogger* srn_logger_new(SrnLoggerConfig *cfg){
    SrnLogger *logger;

    logger = g_malloc0(sizeof(SrnLogger));
    logger->cfg = cfg;

    return logger;
}

void srn_logger_free(SrnLogger *logger){
    g_free(logger);
}

void srn_logger_set_default(SrnLogger *logger) {
    srn_logger = logger;
}

SrnLogger* srn_logger_get_default(void) {
    return srn_logger;
}

SrnRet srn_logger_set_config(SrnLogger *logger, SrnLoggerConfig *cfg) {
    // TODO: free previous?
    logger->cfg = cfg;
    return SRN_OK;
}

void srn_logger_log(SrnLogger *logger, SrnLogLevel lv, bool print_prompt,
        bool new_line, const char *file, const char *func, int line,
        const char *fmt, ...){
    va_list args;
    GString *prompt;
    GString *output;

    if (!file || !is_enabled(logger->cfg, lv, file)) {
        return;
    }

    output = g_string_new("");

    if (print_prompt){
        prompt = g_string_new("");

        if (logger->cfg->prompt_file){
            g_string_append_printf(prompt, "%s", file);
        }
        if (logger->cfg->prompt_function){
            if (logger->cfg->prompt_file){
                prompt = g_string_append(prompt, "::");
            }
            g_string_append_printf(prompt, "%s", func);
        }

        if (logger->cfg->prompt_line){
            if (logger->cfg->prompt_file || logger->cfg->prompt_function){
                g_string_append_printf(prompt, "#L%u", line);
            } else {
                /* If neither file name or function name is printed, print line
                 * number is unnecessary. */
            }
        }

        g_string_printf(output, prompts[logger->cfg->prompt_color][lv], prompt->str);
        g_string_free(prompt, TRUE);
    }

    va_start(args, fmt);
    g_string_append_vprintf(output, fmt, args);
    va_end(args);

    if (new_line){
        output = g_string_append_c(output, '\n');
    }

    if (lv >= LOG_ERROR){
        fprintf(stderr, "%s", output->str);
    } else {
        fprintf(stdout, "%s", output->str);
    }

    g_string_free(output, TRUE);
}

SrnLoggerConfig *srn_logger_config_new(void){
    return g_malloc0(sizeof(SrnLoggerConfig));
}

void srn_logger_config_free(SrnLoggerConfig *cfg){
    if (cfg->debug_targets){
        g_slist_free_full(cfg->debug_targets, g_free);
    }
    if (cfg->info_targets){
        g_slist_free_full(cfg->info_targets, g_free);
    }
    if (cfg->warn_targets){
        g_slist_free_full(cfg->warn_targets, g_free);
    }
    if (cfg->error_targets){
        g_slist_free_full(cfg->error_targets, g_free);
    }

    g_free(cfg);
}

SrnRet srn_logger_config_check(SrnLoggerConfig *cfg){
    return SRN_OK;
}

static bool is_exist(GSList *targets, const char *file){
    GSList *lst;

    lst = targets;
    while (lst){
        /* Only match prefix, so a empty string "" matchs all targets */
        if (g_str_has_prefix(file, lst->data)){
            return TRUE;
        }
        lst = g_slist_next(lst);
    }

    return FALSE;
}

static bool is_enabled(SrnLoggerConfig *cfg, SrnLogLevel lv, const char *file){
    bool enable;

    enable = FALSE;
    switch (lv){
        case LOG_ERROR:
            enable = enable || is_exist(cfg->error_targets, file);
        case LOG_WARN:
            enable = enable || is_exist(cfg->warn_targets, file);
        case LOG_INFO:
            enable = enable || is_exist(cfg->info_targets, file);
        case LOG_DEBUG:
            enable = enable || is_exist(cfg->debug_targets, file);
            break;
        default:
            enable = FALSE;
    }

    return enable;
}
