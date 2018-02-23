# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=6

DESCRIPTION="Modern, beautiful IRC client written in GTK+ 3"
HOMEPAGE="https://github.com/SilverRainZ/srain"
SRC_URI="https://github.com/SilverRainZ/${PN}/archive/${PV}.tar.gz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="debug"

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
	sed -i '/Version=0.06.3/d' data/Srain.desktop || die "sed failed"
	sed -i \
		-e 's,#include <Python.h>,#include <python3.5m/Python.h>,' src/plugin.c \
		-e 's,PY3FLAGS = $(shell pkg-config --cflags python3),PY3FLAGS = $(shell pkg-config --cflags python-3.5m),' \
		-e 's,PY3LIBS = $(shell pkg-config --libs python3),PY3LIBS = $(shell pkg-config --libs python-3.5m),' src/Makefile || die "sed failed"

	default
}

src_configure(){
	econf $(use_enable debug )
}
