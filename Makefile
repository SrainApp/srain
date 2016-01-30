# makefile

.PHONY: init install run
.IGNORE: init

MAKE = make -r
CC = gcc
DEL = rm -rf
LD = ld
CFLAGS = -Wall -Isrc/inc -ggdb -gstabs+
GTK3FLAGS = `pkg-config --cflags gtk+-3.0`
GTK3LIBS = `pkg-config --libs gtk+-3.0`
OBJS = src/srain.c src/i18n.c src/ui.c

build/srain: $(OBJS)
	$(CC) $(CFLAGS) $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

po: $(OBJS)
	xgettext --from-code=UTF-8 -o i18n/srain.pot -k_ -s $(OBJS)
	cd i18n/ && msginit --no-translator -i srain.pot

mo:
	msgfmt i18n/zh_CN.po -o build/locale/zh_CN/LC_MESSAGES/srain.mo

init:
	mkdir -p build > /dev/null
	mkdir -p build/locale/zh_CN/LC_MESSAGES > /dev/null

default: Makefile
	$(MAKE) bulid/srain

run: build/srain
	cd build/ && ./srain

clean: 
	$(DEL) build/*
