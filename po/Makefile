# Makefile

INSTALL = install -v
XGETTEXTFLAG = --sort-output --from-code=UTF-8 \
			   --package-name=$(PACKAGE_NAME) \
			   --package-version=$(PACKAGE_VERSION) \
			   --msgid-bugs-address=$(PACKAGE_EMAIL)

# Cover SRCS
SRCS = $(wildcard ../src/*.c ../src/*/*.c ../build/resources.c)
UIS = $(wildcard ../data/ui/*.glade)
LANGS = $(shell cat LANGS)

srain.pot: $(SRCS) $(UIS)
	xgettext $(XGETTEXTFLAG) -LGlade -ktranslatable -o $@ $(UIS)
	xgettext $(XGETTEXTFLAG) -LC -k_ -o $@ $(SRCS) -j

po: srain.pot LANGS
	for i in $(LANGS); do \
		if test -f po/$$i.po; then \
			msgmerge -U po/$$i.po $<; \
		else \
			msginit -l $$i.UTF-8 --no-translator \
					-i $< -o po/$$i.po; \
		fi; \
	done

mo: LANGS
	for i in $(LANGS); do \
		mkdir -p build/locale/$$i/LC_MESSAGES || ture; \
		msgfmt po/$$i.po -o build/locale/$$i/LC_MESSAGES/srain.mo; \
		$(INSTALL) -Dm644 build/locale/$$i/LC_MESSAGES/srain.mo \
			$(DESTDIR)$(PREFIX)/share/locale/$$i/LC_MESSAGES/srain.mo; \
	done
