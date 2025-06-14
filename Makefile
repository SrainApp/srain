MAKE = make
RM = rm
MKDIR = mkdir
MESON = meson
DBG = gdb

BUILDDIR = builddir
PREFIX = $(PWD)/prefix
FAKE_HOME = $(PREFIX)/home/srain
FAKE_XDG_DATA_DIRS = $(PREFIX)/share:/usr/share:/usr/local/share

.PHONY: default
default:
	$(MAKE) build

.PHONY: build
build: | $(BUILDDIR)
	$(MESON) compile -C $(BUILDDIR)

.PHONY: run
run: install
	unset XDG_CONFIG_HOME XDG_DATA_HOME XDG_CACHE_HOME; \
	export HOME=$(FAKE_HOME); \
	export XDG_DATA_DIRS=$(FAKE_XDG_DATA_DIRS); \
	"$(PREFIX)/bin/srain"

.PHONY: debug
debug: install
	unset XDG_CONFIG_HOME XDG_DATA_HOME XDG_CACHE_HOME; \
	export HOME=$(FAKE_HOME); \
	export XDG_DATA_DIRS=$(FAKE_XDG_DATA_DIRS); \
	$(DBG) $(PREFIX)/bin/srain -ex r -ex bt -ex q

.PHONY: inspect
inspect:
	GTK_DEBUG=interactive \
	GOBJECT_DEBUG=instance-countcd \
	$(MAKE) run

.PHONY: install
install: | $(BUILDDIR) $(PREFIX)
	$(MESON) install -C $(BUILDDIR)

.PHONY: clean
clean:
	$(RM) -rf $(BUILDDIR)
	$(RM) -rf $(PREFIX)

.PHONY: doc
doc:
	xdg-open $(PREFIX)/share/doc/srain/html/index.html

$(BUILDDIR): meson.build | $(PREFIX)
	if [[ "$$OSTYPE" == "darwin"* ]]; then \
		source ./script/macos-pkgconfig-path.sh; \
	fi; \
    if [[ "$$OSTYPE" == "linux-gnu"* ]]; then \
		CC='zig cc' $(MESON) setup --prefix=$(PREFIX) --buildtype=debug $@; \
	else \
		$(MESON) setup --prefix=$(PREFIX) --buildtype=debug -Dapp_indicator=false $@; \
	fi

$(PREFIX):
	$(MKDIR) $@
