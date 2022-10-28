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
 * @file srain.c
 * @brief Main function here
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <stdlib.h>
#include <signal.h>

#include "core/core.h"
#include "ret.h"
#include "i18n.h"
#include "config/config.h"
#include "path.h"
#include "filter/filter.h"
#include "render/render.h"

int c_main(int argc, char *argv[]){
    SrnLogger *logger;
    SrnLoggerConfig *logger_cfg;
    SrnApplication *app;

    ret_init();
    i18n_init();
    srn_filter_init();
    srn_render_init();

    logger_cfg = srn_logger_config_new();
    logger_cfg->warn_targets = g_list_append(
            logger_cfg->warn_targets, g_strdup(""));
    logger_cfg->prompt_function = TRUE;
    logger = srn_logger_new(logger_cfg);
    srn_logger_set_default(logger);

    app = srn_application_new();
    srn_application_run(app, argc, argv);

    srn_render_finalize();
    srn_filter_finalize();
    srn_logger_free(logger);
    ret_finalize();

    return 0;
}
