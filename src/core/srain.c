/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-03-01
 */

#include <stdlib.h>
#include <signal.h>

#include "server.h"

#include "filter.h"
#include "decorator.h"

#include "ret.h"
#include "i18n.h"
#include "prefs.h"
#include "plugin.h"
#include "file_helper.h"

static void quit();

int main(int argc, char *argv[]){
    signal(SIGINT, quit);

    ret_init();
    log_init();
    i18n_init();
    prefs_init();

    create_user_file();

    plugin_init();

    filter_init();
    decorator_init();

    server_init_and_run(argc, argv);

    quit();

    return 0;
}

static void quit(){
    prefs_finalize();
    plugin_finalize();
    server_finalize();
    log_finalize();
    ret_finalize();

    exit(0);
}
