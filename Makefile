# makefile

.PHONY: init install run clean cleanobj
.IGNORE: init

MAKE = make -r
CC = gcc
DEL = rm -rf
LD = ld
CFLAGS = -Wall -Isrc/inc -ggdb -gstabs+
GTK3FLAGS = $$(pkg-config --cflags gtk+-3.0)
GTK3LIBS = $$(pkg-config --libs gtk+-3.0)

TARGET = build/srain
OBJS = build/main.o build/i18n.o build/ui_common.o build/ui_window.o 		\
	   build/ui_chat.o build/ui_msg.o build/ui_detail.o build/ui_image.o	\
	   build/irc_shell.o build/irc_core.o build/socket.o					\
	   build/srain.o

IRCTEST = build/irctest
IRCTEST_OBJS = build/irc_core.o build/irc_shell.o build/irc_test.o build/socket.o

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

build/%.o: src/*/%.c
	$(CC) $(CFLAGS) -c $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

$(IRCTEST): $(IRCTEST_OBJS)
	$(CC) $(CFLAGS) $(GTK3FLAGS) $(GTK3LIBS) $^ -o $@

po: $(OBJS)
	xgettext --from-code=UTF-8 -o po/srain.pot -k_ -s $(OBJS)
	cd po/ && msginit --no-translator -i srain.pot

mo:
	msgfmt po/zh_CN.po -o build/locale/zh_CN/LC_MESSAGES/srain.mo

init:
	mkdir -p build > /dev/null
	mkdir -p build/locale/zh_CN/LC_MESSAGES > /dev/null

default: Makefile
	$(MAKE) $(TARGET)

run: $(TARGET)
	cd build/ && ./srain

dbg: $(TARGET)
	cd build/ && cgdb srain

clean: 
	$(DEL) build/srain

cleanobj:
	$(DEL) build/*.o
