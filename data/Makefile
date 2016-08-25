# Makefile

.PHONY: icons pixmaps themes plugins

INSTALL = install -v

SIZE = 14x14 16x16 22x22 24x24 32x32 36x36 48x48 64x64 72x72 96x96 128x128 192x192 256x256 512x512

default: Srain.desktop icons themes plugins pixmaps 
	$(INSTALL) -Dm644 $< "$(DESTDIR)$(PREFIX)/share/applications/$<"

icons:
	cd icons; \
	for i in *.png; do \
		$(INSTALL) -Dm644 ""$$i"" \
			"$(DESTDIR)$(PREFIX)/share/icons/hicolor/16x16/apps/$$i"; \
	done

themes:
	cd themes; \
	for i in *.css; do \
		$(INSTALL) -Dm644 "$$i" "$(DESTDIR)$(PREFIX)/share/srain/themes/$$i"; \
	done

plugins:
	cd plugins; \
	for i in *.py; do \
		$(INSTALL) -Dm644 "$$i" "$(DESTDIR)$(PREFIX)/share/srain/plugins/$$i"; \
	done

pixmaps:
	cd pixmaps; \
	for i in *.png; do \
		$(INSTALL) -Dm644 "$$i" "$(DESTDIR)$(PREFIX)/share/srain/pixmaps/$$i"; \
	done
	# Install applicaion logos
	cd pixmaps; \
	for i in srain*.png; do \
		for size in $(SIZE); do \
			convert "$$i" -resize $$size "/tmp/srain-tmp.png"; \
			$(INSTALL) -Dm644 "/tmp/srain-tmp.png" \
				"$(DESTDIR)$(PREFIX)/share/icons/hicolor/$$size/apps/$$i" ; \
		done; \
	done; \
	rm "/tmp/srain-tmp.png"
