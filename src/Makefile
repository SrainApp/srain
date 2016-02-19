# makefile
#
# used to compile *.c (which located in . and ./irc ./ui) to object file
# output ot <PROJECT DIR>/build

MAKE = make -r
CC = gcc
CFLAGS = -Wall -Iinc -ggdb -gstabs+
GTK3FLAGS = $$(pkg-config --cflags gtk+-3.0)
GTK3LIBS = $$(pkg-config --libs gtk+-3.0)

SRCS = $(wildcard *.c */*.c)
OBJS = $(patsubst %.c, ../build/%.o, $(notdir $(SRCS)))

default:
	make $(OBJS)

../build/%.o: %.c
	$(CC) $(CFLAGS) -c $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

../build/%.o: */%.c
	$(CC) $(CFLAGS) -c $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@
