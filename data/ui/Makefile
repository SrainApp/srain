# makefile
#
# generate ui resource for app
# compile ui file(*.glade) to <PROJECT DIR>/build/resources.c
# then compile it to <PROJECT DIR>/build/resources.o

GLIB_COMPILE_RESOURCES = glib-compile-resources

default:
	$(MAKE) $(SRCS)

GLADES = $(wildcard *.glade)
SRCS = ../../build/resources.c

$(SRCS): srain.gresource.xml $(GLADES)
	$(GLIB_COMPILE_RESOURCES) srain.gresource.xml --target=$@ --sourcedir=. --generate-source
