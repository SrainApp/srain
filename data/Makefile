# Makefile
#
# Install data files or gererate resources including icons, configure files
# and etc.

.PHONY: resources config icons themes misc plugins 

GLIB_COMPILE_RESOURCES = glib-compile-resources

RES = $(BUILD_DIR)/resources.c

GLADES = $(wildcard ui/*.glade) themes
ICONS = $(wildcard icons/*.*)
THEMESSRC = $(wildcard themes/*.css)
THEMESDST = $(patsubst %, $(DESTDIR)$(PACKAGE_DATA_DIR)/$(PACKAGE)/themes/%, $(notdir $(THEMESSRC)))

default: config icons themes misc plugins

resources: $(RES)

$(RES): srain.gresource.xml $(GLADES) $(ICONS)
	$(GLIB_COMPILE_RESOURCES) $< --target=$@ --sourcedir=. --generate-source

config: builtin.cfg
	$(INSTALL) -Dm644 $< "$(DESTDIR)$(PACKAGE_CONFIG_DIR)/$(PACKAGE)/$<"

icons: icons/scalable/srain.png 
	$(INSTALL) -Dm644 "$<" "$(DESTDIR)$(PACKAGE_DATA_DIR)/icons/hicolor/scalable/apps/$(PACKAGE_APPID).png"

themes: $(THEMESDST)
$(DESTDIR)$(PACKAGE_DATA_DIR)/$(PACKAGE)/themes/%.css: themes/%.css
	$(INSTALL) -Dm644 "$^" "$@"

misc: srain.desktop srain.appdata.xml
	$(INSTALL) -Dm644 "srain.desktop" "$(DESTDIR)$(PACKAGE_DATA_DIR)/applications/$(PACKAGE_APPID).desktop"
	$(INSTALL) -Dm644 "srain.appdata.xml" "$(DESTDIR)$(PACKAGE_DATA_DIR)/metainfo/$(PACKAGE_APPID).appdata.xml"

plugins:
	# TODO
