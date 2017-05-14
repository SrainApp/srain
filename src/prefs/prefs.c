/**
 * @file prefs.c
 * @brief libconfig warpper for srain
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-05-14
 */

#include <libconfig.h>

#include "prefs.h"
#include "srain.h"
#include "log.h"

config_t user_cfg;
config_t builtin_cfg;

void prefs_init(){
    config_init(&user_cfg);
    config_init(&builtin_cfg);
};

void prefs_finalize(){
    config_destroy(&user_cfg);
    config_destroy(&builtin_cfg);
};
