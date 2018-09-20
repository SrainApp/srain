# Makefile

.PHONY: po mo

INSTALL = install -v
XGETTEXTFLAG = --sort-output --from-code=UTF-8 \
			   --package-name=$(PACKAGE_NAME) \
			   --package-version=$(PACKAGE_VERSION) \
			   --msgid-bugs-address=$(PACKAGE_EMAIL)

# Cover SRCS
SRCS = $(wildcard ../src/*.c ../src/*/*.c $(BUILD_DIR)/resources.c)
GLADES = $(wildcard ../data/ui/*.glade)
LANGS = $(shell cat LANGS)

default: mo

srain.pot: $(SRCS) $(GLADES)
	xgettext $(XGETTEXTFLAG) -LGlade -ktranslatable -o $@ $(GLADES)
	xgettext $(XGETTEXTFLAG) -LC -k_ -kN_ -o $@ $(SRCS) -j

po: srain.pot LANGS
	for i in $(LANGS); do \
		if test -f $$i.po; then \
			msgmerge -U $$i.po $<; \
		else \
			msginit -l $$i.UTF-8 --no-translator \
					-i $< -o $$i.po; \
		fi; \
	done

mo: LANGS po
	for i in $(LANGS); do \
		msgfmt $$i.po -o "$(BUILD_DIR)/srain.mo"; \
		$(INSTALL) -Dm644 "$(BUILD_DIR)/srain.mo" \
			"$(DESTDIR)$(PACKAGE_DATA_DIR)/locale/$$i/LC_MESSAGES/srain.mo"; \
	done
