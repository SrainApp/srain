# makefile

.PHONY: init install run
.IGNORE: init

MAKE = make -r
CC = gcc
DEL = rm -rf
LD = ld
CFLAGS = -Wall -Werror -Isrc/inc -m32 -ggdb -gstabs+

build/srain: src/srain.c
	$(CC) $(CFLAGS) $^ -o $@  

init:
	mkdir -p build > /dev/null
	mkdir -p build/locale/zh_CN/LC_MESSAGES > /dev/null

po: src/srain.c
	xgettext --from-code=UTF-8 -o i18n/srain.pot -k_ -s src/srain.c
	cd i18n && msginit --no-translator -i srain.pot

mo:
	msgfmt i18n/zh_CN.po -o build/locale/zh_CN/LC_MESSAGES/srain.mo

default: Makefile
	$(MAKE) bulid/srain

clean: 
	$(DEL) build/*
