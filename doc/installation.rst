============
Installation
============

Srain now can runs on most GNU/Linux distributions and macOS, Windows support in
in plan.

Dependences
===========

=================== =============== =======
Name                Notes           Version
=================== =============== =======
coreutils           building
make                building
gcc                 building
pkg-config          building
imagemagick         building
gettext
glib2
glib-networking     TLS support
gtk+3                               >= 3.16
python3                             >= 3.2
libcurl
libnotify
libconfig                           >= 1.5
python3-sphinx      document
=================== =============== =======

Manual Installation
===================

You should install the aboved dependencies on your platfrom before manual
installation.

Install Dependences
-------------------

Arch Linux
~~~~~~~~~~

.. code-block:: console

    # pacman -S ...

openSUSE
~~~~~~~~

.. code-block:: console

    # zypper in ...

Ubuntu & Debian
~~~~~~~~~~~~~~~

.. code-block:: console

    # apt-get install ...

macOS
~~~~~

.. code-block:: console

    $ brew install coreutils gcc pkg-config imagemagick # Building
    $ brew install glib glib-networking gettext gtk+3 python3 curl libnotify libconfig

Compile & Install
-------------------

After installing the aboved dependencies, complile and install:

.. code-block:: console

    $ ./configure --prefix=/usr/local --config-dir=/usr/local/etc
    $ make
    # make install

.. note::

    The configure script **doesn't** check any dependience. So you should make
    sure that you have fully installed all dependencies.

Install documents

.. code-block:: console

    $ make doc
    # make install-doc

Distribution Package
====================

Arch Linux
----------

AUR Package `srain`_ and `srain-git`_ (git version) are available on AUR,
it is quite easy to install using yaourt:

.. code-block:: console

    $ yaourt -S srain
    $ yaourt -S srain-git # git version

If you are the user of `Arch Linux CN Repository`_, try:

.. code-block:: console

    # pacman -S archlinuxcn/srain
    # pacman -S archlinuxcn/srain-git # git version

.. _srain: https://aur.archlinux.org/packages/srain
.. _srain-git: https://aur.archlinux.org/packages/srain-git
.. _Arch Linux CN Repository: https://www.archlinuxcn.org/archlinux-cn-repo-and-mirror

Gentoo
------

`rtlanceroad`_ maintains `Gentoo ebuilds for Srain`_, please refer to it for
more details.

.. _rtlanceroad: https://aur.archlinux.org/packages/srain
.. _Gentoo ebuilds for Srain: https://github.com/rtlanceroad/gentoo-srain

Debian
------

`dfceaef`_ has written `Debian package script for srain`_, but it is already
**out of date**, I will be glad if anyone can fix it.

.. _dfceaef: https://github.com/yangfl
.. _Debian package script for Srain: https://github.com/SilverRainZ/srain/tree/debian/debian
