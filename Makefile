# main makefile
# make without any argument to generate `buile/srain`
# make dgb debug app with cgdb

.PHONY: init install run clean cleanobj
.IGNORE: init

MAKE = make -r
DEL = rm -rf
CC = gcc
CFLAGS = -Wall -Isrc/inc -ggdb -gstabs+
GTK3FLAGS = $$(pkg-config --cflags gtk+-3.0)
GTK3LIBS = $$(pkg-config --libs gtk+-3.0)
PY3FLAGS = $$(pkg-config --cflags python3)
PY3LIBS = $$(pkg-config --libs python3)

TARGET = build/srain
SRCS = $(wildcard src/*.c src/*/*.c build/resources.c)
OBJS = $(patsubst %.c, build/%.o, $(notdir $(SRCS)))

default: Makefile
	cd src; $(MAKE)			# compile c code
	cd data/ui; $(MAKE)		# compile resources
	$(MAKE) $(TARGET)

init:
	mkdir -p build > /dev/null
	mkdir -p build/locale/zh_CN/LC_MESSAGES > /dev/null

run: default
	cp srainrc.example build/srainrc
	cp data/theme/*.css build/
	cp data/img/*.png build/
	cp plugin/*.py build/
	cd build/ && ./srain

dbg: $(TARGET)
	cd build/ && cgdb srain

clean: 
	$(DEL) build/srain

cleanobj:
	$(DEL) build/*.o

# target `po` and `mo` are no used recently, ignore them
po: $(OBJS)
	xgettext --from-code=UTF-8 -o po/srain.pot -k_ -s $(OBJS)
	cd po/ && msginit --no-translator -i srain.pot

mo:
	msgfmt po/zh_CN.po -o build/locale/zh_CN/LC_MESSAGES/srain.mo

# compile multiple object file to execute file
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(GTK3FLAGS) $(GTK3LIBS) $(PY3FLAGS) $(PY3LIBS) $^ -o $@
