# makefile
#
# generate ui resource for app
# compile ui file(*.glade) to <PROJECT DIR>/build/resources.c
# then compile it to <PROJECT DIR>/build/resources.o

MAKE = make -r
CC = gcc
CFLAGS = -Wall -ggdb -gstabs+
GTK3FLAGS = $$(pkg-config --cflags gtk+-3.0)
GTK3LIBS = $$(pkg-config --libs gtk+-3.0)
GLIB_COMPILE_RESOURCES = glib-compile-resources

default:
	$(MAKE) $(OBJS)

SRCS = ../../build/resources.c
OBJS = $(patsubst %.c, %.o, $(SRCS))

GLADES = $(wildcard *.glade)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

$(SRCS): srain.gresource.xml $(GLADES)
	$(GLIB_COMPILE_RESOURCES) srain.gresource.xml --target=$@ --sourcedir=. --generate-source
