#include <stdio.h>
#include <string.h>
#include "log.h"
#include "srain.h"

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
            srain_cmd("*server*", line);
        }
    }

    if (line) free(line);
    fclose(fp);
    return 0;
}
