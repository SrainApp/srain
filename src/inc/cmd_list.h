#ifndef __CMD_LIST_H
#define __CMD_LIST_H

/* cmd_list is a list of available commands of this app,
 * auto-generated from the src/server/server_cmd.c.
 */
static const char *cmd_list[] = {
#include "cmd_list.autogen"
NULL
};

#endif /* __CMD_LIST_H */

