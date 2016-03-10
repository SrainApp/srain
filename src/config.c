/**
 * @file config.c
 * @brief auto command config reader
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * read $XDG_CONFIG_HOME/srain/srainrc, call `srain_cmd` for every line
 */

#define __LOG_ON

#include <stdio.h>
#include <string.h>
#include "log.h"
#include "srain.h"
#include "srain_magic.h"

int config_read(){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("./srainrc", "r");
    if (!fp){
        ERR_FR("could no open `.srainrc`");
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        if (line){
            strtok(line, "\n");
            LOG_FR("read: %s", line);
            srain_cmd(SERVER, line);
        }
    }

    if (line) free(line);
    fclose(fp);
    return 0;
}
