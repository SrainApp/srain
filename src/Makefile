# Makefile
#
# Complie C source codes and output object files to $(BUILD_DIR), then links
# them together to get a executable $(TARGET).

CC = gcc
DEFS = -D PACKAGE='"$(PACKAGE)"' \
	   -D PACKAGE_NAME='"$(PACKAGE_NAME)"' \
	   -D PACKAGE_VERSION='"$(PACKAGE_VERSION)"' \
	   -D PACKAGE_BUILD='"$(PACKAGE_BUILD)"' \
	   -D PACKAGE_AUTHOR='"$(PACKAGE_AUTHOR)"' \
	   -D PACKAGE_EMAIL='"$(PACKAGE_EMAIL)"' \
	   -D PACKAGE_DESC='"$(PACKAGE_DESC)"' \
	   -D PACKAGE_WEBSITE='"$(PACKAGE_WEBSITE)"' \
	   -D PACKAGE_DATA_DIR='"$(PACKAGE_DATA_DIR)"' \
	   -D PACKAGE_CONFIG_DIR='"$(PACKAGE_CONFIG_DIR)"' \
	   -D PACKAGE_COPYRIGHT_DATES='"$(PACKAGE_COPYRIGHT_DATES)"'
GTK3FLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK3LIBS = $(shell pkg-config --libs gtk+-3.0)
PY3FLAGS = $(shell pkg-config --cflags python3)
PY3LIBS = $(shell pkg-config --libs python3)
LIBCONFIGFLAGS = $(shell pkg-config --cflags libconfig)
LIBCONFIGLIBS = $(shell pkg-config --libs libconfig)
LIBSOUPFLAGS = $(shell pkg-config --cflags libsoup-2.4)
LIBSOUPLIBS = $(shell pkg-config --libs libsoup-2.4)

CFLAGS += -std=gnu99 -O2 -Wall -Iinc -Wno-deprecated-declarations \
		  $(DEFS) $(GTK3FLAGS) $(PY3FLAGS) $(LIBIRCCLINETFLAGS) \
		  $(LIBCONFIGFLAGS) $(LIBSOUPFLAGS) $(DBGFLAGS)

LDFLAGS += $(GTK3LIBS) $(PY3LIBS) $(LIBIRCCLINETLIBS) \
		   $(LIBCONFIGLIBS) $(LIBSOUPLIBS) $(DBGFLAGS)

SRCS = $(wildcard *.c */*.c $(BUILD_DIR)/resources.c)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))

default: sui/glade/Makefile
	$(MAKE) -C sui/glade
	$(MAKE) $(TARGET)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILD_DIR)/%.o: */%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)
