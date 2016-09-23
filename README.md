<img src="./data/icons/raw/srain.png" width=8%> Srain
=====

**Note:** Srain is still under development.

It does not look like an IRC client :-)

# Dependencies

- make          (makedepends)
- gcc           (makedepends)
- pkg-config    (makedepends)
- gettext       (makedepends)
- imagemagick   (makedepends)
- gtk >= 3.16
- python >= 3.2
- libcurl
- libircclient >= 1.8
- libnotify

# Build & Debug

    mkdir build
    ./configure --prefix=$PWD/build --enable-debug
    make
    make run

# Install

## Arch Linux

    yaourt -S srain-git # git version
    yaourt -S srain     # latest release

or

    # Add archlinuxcn mirror, then
    pacman -S archlinuxcn/srain-git

## Gentoo

Try [srain-0.05.ebuild](https://github.com/rtlanceroad/gentoo-ebuilds/blob/master/net-irc/srain/srain-0.05.ebuild),
thx to @xeirrr.

## For other linux distributions:

    # Intall the above dependencies, then
    # Note: the configure script doesn't check dependiencies.
    mkdir build
    ./configure --prefix=/usr/local --disable-debug
    make
	make DESTDIR=/ install

# Feature

- Beautiful User Interface
- Relay bot message transform
- Preview image from URL
- Get avatar according to user's real name (plugin)
- Auto upload image to pastebin (plugin)

# Screenshot

As you see, its theme is inspired by Telegram Desktop.

![screenshot](http://img.tjm.moe/47/ceece073d29563da0c22ab6e8e8c3cdc534113.png)

# License

GPL v3
