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
CURLLIBS = $$(pkg-config --libs libcurl)
DESTDIR = $$(if [ -z $$DESTDIR ]; then echo $(PWD)/build; fi)

TARGET = build/srain
SRCS = $(wildcard src/*.c src/*/*.c build/resources.c)
OBJS = $(patsubst %.c, build/%.o, $(notdir $(SRCS)))

default: Makefile src/Makefile data/ui/Makefile
	cd src; $(MAKE) DESTDIR=$(DESTDIR)			# compile c code
	cd data/ui; $(MAKE)							# compile resources
	$(MAKE) $(TARGET)

init:
	mkdir -p build > /dev/null
	mkdir -p build/locale/zh_CN/LC_MESSAGES > /dev/null

install:
	install -Dm755 "build/srain" "$(DESTDIR)/bin/srain"
	mkdir -p "$(DESTDIR)/share/srain/img"
	cd data && for png in img/*.png; do echo $$png; install -Dm644 "$$png" "$(DESTDIR)/share/srain/$$png"; done
	mkdir -p "$(DESTDIR)/share/srain/theme"
	cd data && for css in theme/*.css; do echo $$css; install -Dm644 "$$css" "$(DESTDIR)/share/srain/$$css"; done
	mkdir -p "$(DESTDIR)/share/srain/plugin"
	for py in plugin/*.py; do echo $$py; install -Dm644 "$$py" "$(DESTDIR)/share/srain/$$py"; done

run: default
	$(MAKE) DESTDIR=$(DESTDIR)
	cp srainrc.example build/share/srain/srainrc
	$(MAKE) DESTDIR=$(DESTDIR) install
	build/bin/srain

dbg: $(TARGET)
	cd build/ && cgdb srain

clean: 
	$(DEL) build/bin/srain

cleanobj:
	$(MAKE) clean
	$(DEL) build/*.o

# target `po` and `mo` are no used recently, ignore them
po: $(OBJS)
	xgettext --from-code=UTF-8 -o po/srain.pot -k_ -s $(OBJS)
	cd po/ && msginit --no-translator -i srain.pot

mo:
	msgfmt po/zh_CN.po -o build/locale/zh_CN/LC_MESSAGES/srain.mo

# compile multiple object file to execute file
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(GTK3FLAGS) $(PY3FLAGS) $^ -o $@ $(GTK3LIBS) $(PY3LIBS) $(CURLLIBS)
