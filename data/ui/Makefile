# Makefile
#
# Generate ui resources for srain application.
#
# Compile ui file(*.glade) to $(BUILD_DIR)/resources.c, then compile it to
# $(BUILD_DIR)/resources.o

GLIB_COMPILE_RESOURCES = glib-compile-resources

default:
	$(MAKE) $(SRCS)

GLADES = $(wildcard *.glade)
SRCS = $(BUILD_DIR)/resources.c

$(SRCS): srain.gresource.xml $(GLADES)
	$(GLIB_COMPILE_RESOURCES) srain.gresource.xml --target=$@ --sourcedir=. --generate-source
