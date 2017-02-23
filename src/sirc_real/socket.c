 /**
 * @file socket.c
 * @brief Simple socket wrapper
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 *
 * Copide from <https://github.com/Themaister/simple-irc-bot>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "srain.h"
#include "log.h"

int sck_get_socket(const char *host, int port){
    int rc;
    int fd;
    char sport[6];
    struct addrinfo hints, *res;

    snprintf(sport, sizeof(sport), "%d", port);
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rc = getaddrinfo(host, sport, &hints, &res) ) < 0 ){
        ERR_FR("getaddrinfo() error: %s", gai_strerror(rc));
        return SRN_ERR;
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0){
        ERR_FR("Couldn't get socket");
        goto bad;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0){
        ERR_FR("Couldn't connect");
        goto bad;
    }

    freeaddrinfo(res);
    return fd;

bad:
    freeaddrinfo(res);
    return -1;
}

int sck_set_timeout(int fd, double second){
    struct timeval tv;

    tv.tv_sec = (int) second;
    second -= (int) second;
    tv.tv_usec = (int) (second * 1e6);

    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(tv));
}

int sck_send(int fd, const char *data, size_t size){
    size_t written = 0;
    int rc;

    while (written < size){
        rc = send(fd, data + written, size - written, 0);
        if (rc <= 0){
            ERR_FR("Socket error");
            return SRN_ERR;
        }
        written += rc;
    }

    return written;
}

int sck_recv(int fd, char *buf, size_t size){
    int rc;

    rc = recv(fd, buf, size, 0);
    if (rc <= 0){
        ERR_FR("socket error");
        return -1;
    }

    return rc;
}

int sck_readline(int fd, char *buf, size_t size){
    int rc;
    int i = 0;
    char byte;

    while (i < size){
        rc = recv(fd, &byte, 1, 0);
        if (rc < 0) return SRN_ERR;
        if (byte == '\r'){
            rc = recv(fd, &byte, 1, MSG_PEEK);
            if (rc < 0) return SRN_ERR;
            /* End of a IRC message */
            if (byte == '\n'){
                recv(fd, &byte, 1, 0);
                buf[i++] = '\0';
                return i;
            } else {
                byte = '\r';
            }
        }
        buf[i++] = byte;
    }

    ERR_FR("IRC message to long");
    return SRN_ERR;
}
