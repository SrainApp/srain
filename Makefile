# Makefile

.PHONY: init install run clean cleanobj todo
.IGNORE: init

# Export all variables
export

MAKE = make
DEL = rm -rf
CC = gcc
INC = -Isrc/inc
DBGFLAGS = -O0 -ggdb -gstabs+
CFLAGS = -Wall -DMETA_PACKAGE_DATA_DIRS=\"$(PREFIX)\"
GTK3FLAGS = `pkg-config --cflags gtk+-3.0`
GTK3LIBS = `pkg-config --libs gtk+-3.0`
PY3FLAGS = `pkg-config --cflags python3`
PY3LIBS = `pkg-config --libs python3`
CURLLIBS = `pkg-config --libs libcurl`
PREFIX = $(PWD)/build

TARGET = build/srain
SRCS = $(wildcard src/*.c src/*/*.c build/resources.c)
OBJS = $(patsubst %.c, build/%.o, $(notdir $(SRCS)))

default: Makefile src/Makefile data/ui/Makefile
	$(MAKE) -C src
	$(MAKE) -C data/ui
	$(MAKE) $(TARGET)

init:
	mkdir -p build || true
	mkdir -p build/locale/zh_CN/LC_MESSAGES || true

install:
	install -Dm755 "build/srain" "$(DESTDIR)$(PREFIX)/bin/srain"
	cd data/icons; \
		for png in *.png; do \
			install -Dm644 "$$png" \
				"$(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/$$png"; \
		done
	cd data/pixmaps; \
		for png in *.png; do \
			install -Dm644 "$$png" \
				"$(DESTDIR)$(PREFIX)/share/srain/pixmaps/$$png"; \
		done
	cd data/themes; \
		for css in *.css; do \
			install -Dm644 "$$css" \
				"$(DESTDIR)$(PREFIX)/share/srain/themes/$$css"; \
		done
	cd data/plugins; \
	for py in *.py; do \
		install -Dm644 "$$py" \
		"$(DESTDIR)$(PREFIX)/share/srain/plugins/$$py"; \
		done

run: default
	$(MAKE) PREFIX=$(PWD)/build install
	$(MAKE) PREFIX=$(PWD)/build install
	cp srainrc.example build/srainrc
	cd build; XDG_DATA_DIRS=$(PWD)/build/share:/usr/share/ ./bin/srain

dbg: $(TARGET)
	cd build/ && cgdb srain

clean:
	$(DEL) build/srain
	$(DEL) build/*.o
	$(DEL) build/*.c
	$(DEL) build/bin
	$(DEL) build/share

# Target `po` and `mo` are no used recently, ignore them
# TODO: out-of-date
po: $(OBJS)
	xgettext --from-code=UTF-8 -o po/srain.pot -k_ -s $(OBJS)
	cd po/ && msginit --no-translator -i srain.pot

# TODO: out-of-date
mo:
	msgfmt po/zh_CN.po -o build/locale/zh_CN/LC_MESSAGES/srain.mo

todo:
	grep --color=auto -r "TODO" src/

# Compile multiple object files to execute file
$(TARGET): $(OBJS)
	$(CC) $(INC) $(CFLAGS) $(GTK3FLAGS) $(PY3FLAGS) $^ -o $@ $(GTK3LIBS) $(PY3LIBS) $(CURLLIBS)
