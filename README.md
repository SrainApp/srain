Srain
=====

It does not look like an IRC client :-)

# Dependencies

- make          (makedepends)
- gcc           (makedepends)
- pkg-config    (makedepends)
- gettext       (makedepends)
- gtk >= 3.19
- glib2
- python >= 3.2
- libcurl
- libircclient >= 1.8

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

    mkdir build
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

![screenshot](http://img.tjm.moe/c5/1e91eced2cb59498a2cbe32fcf7d797f6f626d.png)
