#ifndef __SOCKET_H
#define __SOCKET_H

#include <stdlib.h>

int sck_get_socket(const char *host, int port);
int sck_set_timeout(int fd, double second);
int sck_send(int socket, const char* data, size_t size);
int sck_sendf(int socket, const char* fmt, ...);
int sck_recv(int socket, char* buf, size_t size);
int sck_readline(int fd, char *buf, size_t size);

#endif
