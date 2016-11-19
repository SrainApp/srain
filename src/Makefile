# makefile
#
# used to compile *.c (which located in . and ./irc ./ui) to object file
# output ot <PROJECT DIR>/build
#
CC = gcc
INSTALL = install -v
TEST_FLAG = NO_TEST
DEFS = -D PACKAGE='"$(PACKAGE)"' \
	   -D PACKAGE_NAME='"$(PACKAGE_NAME)"' \
	   -D PACKAGE_VERSION='"$(PACKAGE_VERSION)"' \
	   -D PACKAGE_AUTHOR='"$(PACKAGE_AUTHOR)"' \
	   -D PACKAGE_EMAIL='"$(PACKAGE_EMAIL)"' \
	   -D PACKAGE_DESC='"$(PACKAGE_DESC)"' \
	   -D PACKAGE_WEBSITE='"$(PACKAGE_WEBSITE)"' \
	   -D PACKAGE_DATA_DIR='"$(PACKAGE_DATA_DIR)"' \
	   -D $(TEST_FLAG)
CFLAGS = -O2 -Wall -I$(PWD)/src/inc -Wno-deprecated-declarations
GTK3FLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK3LIBS = $(shell pkg-config --libs gtk+-3.0)
PY3FLAGS = $(shell pkg-config --cflags python3)
PY3LIBS = $(shell pkg-config --libs python3)
CURLFLAGS = $(shell pkg-config --cflags libcurl)
CURLLIBS = $(shell pkg-config --libs libcurl)
# LIBIRCCLINETFLAGS = $(shell pkg-config --cflags libircclient)
# LIBIRCCLINETLIBS = $(shell pkg-config --libs libircclient)
LIBIRCCLINETFLAGS = -lircclient
LIBIRCCLINETLIBS = -I/usr/include/libircclient
LIBNOTIFYFLAGS = $(shell pkg-config --cflags libnotify)
LIBNOTIFYLIBS = $(shell pkg-config --libs libnotify)

CFLAGS += $(DEFS) $(GTK3FLAGS)  $(GTK3LIBS) $(PY3FLAGS) $(PY3LIBS) \
		  $(CURLFLAGS) $(CURLLIBS) $(LIBIRCCLINETFLAGS) $(LIBIRCCLINETLIBS) \
		  $(LIBNOTIFYFLAGS) $(LIBNOTIFYLIBS)

TARGET = ../build/srain
SRCS = $(wildcard *.c */*.c ../build/resources.c)
OBJS = $(patsubst %.c, ../build/%.o, $(notdir $(SRCS)))

default:
	./cmd_list.sh --gen
	make $(TARGET)
	./cmd_list.sh --restore

../build/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

../build/%.o: */%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
