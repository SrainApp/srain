# Makefile
#
# Install data files including icons, configure files, pixmaps and etc.

.PHONY: icons pixmaps themes plugins

SIZE = 14x14 16x16 22x22 24x24 32x32 36x36 48x48 64x64 72x72 96x96 128x128 192x192 256x256 512x512

default: im.srain.Srain.desktop icons themes plugins config
	$(INSTALL) -Dm644 "im.srain.Srain.desktop" "$(DESTDIR)$(PACKAGE_DATA_DIR)/share/applications/im.srain.Srain.desktop"
	$(INSTALL) -Dm644 "im.srain.Srain.appdata.xml" "$(DESTDIR)$(PACKAGE_DATA_DIR)/share/metainfo/im.srain.Srain.appdata.xml"

icons:
	cd icons; \
	for size in $(SIZE); do \
		if [ -d "$$size" ]; then \
			cd $$size; \
			for i in *.png; do \
				$(INSTALL) -Dm644 "$$i" \
					"$(DESTDIR)$(PACKAGE_DATA_DIR)/share/icons/hicolor/$$size/apps/$$i"; \
			done; \
			cd ..; \
		fi; \
	done
	# Convert raw image to specified size
	cd icons/raw; \
	for i in *.png; do \
		for size in $(SIZE); do \
			convert "$$i" -resize $$size "/tmp/srain-tmp.png"; \
			$(INSTALL) -Dm644 "/tmp/srain-tmp.png" \
				"$(DESTDIR)$(PACKAGE_DATA_DIR)/share/icons/hicolor/$$size/apps/$$i"; \
		done; \
	done; \
	rm "/tmp/srain-tmp.png"

themes:
	cd themes; \
	for i in *.css; do \
		$(INSTALL) -Dm644 "$$i" "$(DESTDIR)$(PACKAGE_DATA_DIR)/share/srain/themes/$$i"; \
	done

plugins:
	# TODO

config: builtin.cfg
	$(INSTALL) -Dm644 "builtin.cfg" "$(DESTDIR)$(PACKAGE_CONFIG_DIR)/srain/builtin.cfg"
