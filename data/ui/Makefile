# makefile
#
# generate ui resource for app
# compile ui file(*.glade) to <PROJECT DIR>/build/resources.c
# then compile it to <PROJECT DIR>/build/resources.o

GLIB_COMPILE_RESOURCES = glib-compile-resources

default:
	$(MAKE) $(OBJS)

SRCS = ../../build/resources.c
OBJS = $(patsubst %.c, %.o, $(SRCS))

GLADES = $(wildcard *.glade)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $^ -o $@

$(SRCS): srain.gresource.xml $(GLADES)
	$(GLIB_COMPILE_RESOURCES) srain.gresource.xml --target=$@ --sourcedir=. --generate-source
