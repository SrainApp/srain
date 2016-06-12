Srain
=====
[![Travis](https://img.shields.io/travis/LastAvenger/srain.svg?maxAge=2592000)](https://travis-ci.org/LastAvenger/srain)
[![Github All Releases](https://img.shields.io/github/downloads/LastAvenger/srain/total.svg?maxAge=2592000)](https://github.com/LastAvenger/srain/releases)

It does not look like an IRC client :-)

# Dependencies

- make          (makedepends)
- gcc           (makedepends)
- libcurl       (makedepends)
- pkg-config    (makedepends)
- gettext       (makedepends)
- gtk >= 3.16
- glib2
- python >= 3.2

# Build & Debug

    mkdir build
    ./configure --prefix=$PWD/build --enable-debug
    make
    make run

# Install

## For Arch Linux users

    yaourt -S srain-git # git version
    yaourt -S srain     # latest release

## For other linux distributions:

    ./configure --prefix=/usr/local --disable-debug
    make
	make DESTDIR=/ install

# Feature

- Relay bot message transform
- Preview image from URL
- Get avatar from github (plugin)
- Auto upload image to pastebin (plugin)

# Screenshot

As you see, its theme is inspired by Telegram Desktop.

![screenshot](https://img.vim-cn.com/4f/59a3f20a2f7402d94b6759b860e37dac5bf843.png)
