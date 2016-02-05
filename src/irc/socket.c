/* socket.c
 * This file is modified from <https://github.com/Themaister/simple-irc-bot>
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

int get_socket(const char *host, const char *port){
   int rc;
   int fd;
   struct addrinfo hints, *res;

   memset(&hints, 0, sizeof(hints));

   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;

   if ((rc = getaddrinfo(host, port, &hints, &res) ) < 0 ){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
      return -1;
   }

   fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if (fd < 0){
      fprintf(stderr, "Couldn't get socket.\n");
      goto bad;
   }

   if (connect(fd, res->ai_addr, res->ai_addrlen) < 0){
      fprintf(stderr, "Couldn't connect.\n");
      goto bad;
   }

   freeaddrinfo(res);
   return fd;

bad:
   freeaddrinfo(res);
   return -1;
}

int sck_send(int fd, const char *data, size_t size){
   size_t written = 0;
   int rc;

   while (written < size){
      rc = send(fd, data + written, size - written, 0);
      if (rc <= 0){
         return -1;
      }
      written += rc;
   }

   return written;
}

int sck_sendf(int fd, const char *fmt, ...){
   char send_buf[512];
   int send_len;
   va_list args;

   if (strlen(fmt) != 0 ){
      // Format the data
      va_start(args, fmt);
      send_len = vsnprintf(send_buf, sizeof (send_buf), fmt, args);
      va_end(args);

      // Clamp the chunk
      if (send_len > 512) 
         send_len = 512;
   
      if (sck_send(fd, send_buf, send_len ) <= 0) 
         return -1;
      return send_len;
   }
   return 0;
}

int sck_recv(int fd, char *buffer, size_t size){
   int rc;

   rc = recv(fd, buffer, size, 0);
   if (rc <= 0){
      return -1;
   }

   return rc;
}
