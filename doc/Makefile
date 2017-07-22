# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = python -msphinx
SPHINXPROJ    = Srain
SOURCEDIR     = .
BUILDDIR      = $(BUILD_DIR)/doc

default:
	make html

view:
	xdg-open "$(BUILDDIR)/html/index.html"

install: install-rst install-html

install-rst:
	for rst in *.rst; do \
		echo $$rst; \
		$(INSTALL) -Dm644 $$rst \
		"$(DESTDIR)$(PACKAGE_DATA_DIR)/share/doc/$(PACKAGE)/rst/$$rst"; \
	done;

install-html:
	cd $(BUILDDIR)/html; \
	for html in *.html; do \
		$(INSTALL) -Dm644 $$html \
		"$(DESTDIR)$(PACKAGE_DATA_DIR)/share/doc/$(PACKAGE)/html/$$html"; \
	done; \
	cp -r _static "$(DESTDIR)$(PACKAGE_DATA_DIR)/share/doc/$(PACKAGE)/html";

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: default Makefile help default view install install-rst install-html

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
