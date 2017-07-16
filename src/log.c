/**
 * @file log.c
 * @brief A logger which configured via configure file
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-06-24
 *
 */

#include <stdio.h>
#include <glib.h>

#include "srain.h"
#include "prefs.h"
#include "log.h"

static LogPrefs *log_prefs = NULL;

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

static LogPrefs *log_prefs_new();
static void log_prefs_free(LogPrefs *prefs);

static bool is_exist(GSList *files, const char *file);
static bool is_enabled(LogLevel lv, const char *file);

void log_init(){
    log_prefs = log_prefs_new();

   /* Inital preset for prefs moduele */
    log_prefs->debug_targets = g_slist_append(log_prefs->debug_targets, g_strdup("*"));
}

SrnRet log_read_prefs(){
    log_prefs_free(log_prefs);
    log_prefs = log_prefs_new();

    return prefs_read_log_prefs(log_prefs);
}

void log_finalize(){
    log_prefs_free(log_prefs);
}

static LogPrefs *log_prefs_new(){
    LogPrefs *prefs;

    prefs = g_malloc0(sizeof(LogPrefs));

    return prefs;
}

static void log_prefs_free(LogPrefs *prefs){
    if (prefs->debug_targets){
        g_slist_free_full(prefs->debug_targets, g_free);
    }
    if (prefs->info_targets){
        g_slist_free_full(prefs->info_targets, g_free);
    }
    if (prefs->warn_targets){
        g_slist_free_full(prefs->warn_targets, g_free);
    }
    if (prefs->error_targets){
        g_slist_free_full(prefs->error_targets, g_free);
    }

    g_free(prefs);
}

/**
 * @brief log_printp Print log in a line(suffix "ln") with prompt suffix "p"
 *
 * @param lv Log level
 * @param print_prompt
 * @param new_line
 * @param file File name, shoule be the value of __FILE__
 * @param func Function name, shoule be the value of __FUNCTION__
 * @param line Line number in file, should be the value of __LINE__
 * @param fmt Format string
 * @param ...
 */
void log_print(LogLevel lv, bool print_prompt, bool new_line,
        const char *file, const char *func, int line,
        const char *fmt, ...){
    va_list args;
    GString *prompt;
    GString *output;

    if (!file || !is_enabled(lv, file)) {
        return;
    }

    output = g_string_new("");

    if (print_prompt){
        prompt = g_string_new("");

        if (log_prefs->prompt_file){
            g_string_append_printf(prompt, "%s", file);
        }
        if (log_prefs->prompt_function){
            if (log_prefs->prompt_file){
                prompt = g_string_append(prompt, "::");
            }
            g_string_append_printf(prompt, "%s", func);
        }

        if (log_prefs->prompt_line){
            if (log_prefs->prompt_file || log_prefs->prompt_function){
                g_string_append_printf(prompt, "#L%u", line);
            } else {
                /* If neither file name or function name is printed, print line
                 * number is unnecessary. */
            }
        }

        g_string_printf(output, prompts[log_prefs->prompt_color][lv], prompt->str);
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

static bool is_exist(GSList *targets, const char *file){
    GSList *lst;

    lst = targets;
    while (lst){
        /* A "*" matchs all targets */
        if (g_strcmp0(lst->data, "*") == 0){
            return TRUE;
        }
        if (g_strcmp0(lst->data, file) == 0){
            return TRUE;
        }
        lst = g_slist_next(lst);
    }

    return FALSE;
}

static bool is_enabled(LogLevel lv, const char *file){
    bool enable;

    enable = FALSE;
    switch (lv){
        case LOG_ERROR:
            enable = enable || is_exist(log_prefs->error_targets, file);
        case LOG_WARN:
            enable = enable || is_exist(log_prefs->warn_targets, file);
        case LOG_INFO:
            enable = enable || is_exist(log_prefs->info_targets, file);
        case LOG_DEBUG:
            enable = enable || is_exist(log_prefs->debug_targets, file);
            break;
        default:
            enable = FALSE;
    }

    return enable;
}
