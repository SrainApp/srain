# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

inherit git-r3

DESCRIPTION="Modern, beautiful IRC client written in GTK+ 3"
HOMEPAGE="https://github.com/SilverRainZ/srain"
EGIT_REPO_URI="https://github.com/SilverRainZ/srain.git"
IUSE="debug"

LICENSE="GPL-3"
SLOT="0"

DEPEND="
	dev-libs/libconfig
	dev-python/sphinx
	media-gfx/imagemagick
	net-libs/libsoup
"
RDEPEND="${DEPEND}
	dev-python/requests
	>=x11-libs/gtk+-3.16.7
	x11-libs/libnotify
"

src_prepare(){
	mkdir build
	sed -i \
		-e 's,#include <Python.h>,#include <python3.5m/Python.h>,' src/plugin.c \
		-e 's,PY3FLAGS = $(shell pkg-config --cflags python3),PY3FLAGS = $(shell pkg-config --cflags python-3.5m),' \
		-e 's,PY3LIBS = $(shell pkg-config --libs python3),PY3LIBS = $(shell pkg-config --libs python-3.5m),' src/Makefile || die "sed failed"

	default
}

src_configure(){
	econf $(use_enable debug )
}
