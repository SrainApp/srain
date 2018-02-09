============
Installation
============

Srain can run on most GNU/Linux distributions and macOS, Windows support is in
the plan.

.. contents::
    :local:
    :depth: 3
    :backlinks: none

Dependencies
============

=================== =========================== =======
Name                Notes                       Version
=================== =========================== =======
coreutils           building
make                building
gcc                 building
pkg-config          building
imagemagick         building
gettext
glib2
glib-networking     TLS support
gtk+3                                           >= 3.16
python3                                         >= 3.2
libcurl             will be removed soon
libsoup
libnotify
libconfig                                       >= 1.5
python3-sphinx      document
python3-urllib3     avatar and pastebin support
python3-request     avatar and pastebin support
=================== =========================== =======

Manual Installation
===================

You should install the aboved dependencies on your platfrom before manual
installation.

Install Dependencies
--------------------

.. note::

    The accurate package name is depending on platform,
    the following commands just for reference.

.. warning::

    The new dependency ``libsoup`` hasn't included in some of the following
    packages, for it is hard to maintain a list of packages for every platform.
    Sorry!

Arch Linux
~~~~~~~~~~

.. literalinclude:: archlinux.sh
    :language: shell

openSUSE
~~~~~~~~

.. literalinclude:: opensuse.sh
    :language: shell


Debian & Ubuntu
~~~~~~~~~~~~~~~

.. literalinclude:: debian.sh
    :language: shell

macOS
~~~~~

.. literalinclude:: macos.sh
    :language: shell

Compile & Install
-------------------

After installing the aboved dependencies, download the source code:

Get stable(require ``wget`` and ``tar``):

.. code-block:: console

    $ wget https://github.com/SilverRainZ/srain/archive/0.06.3.tar.gz
    $ tar -xvzf 0.06.3.tar.gz
    $ cd srain-0.06.3

Get git version(require ``git``):

.. code-block:: console

    $ git clone https://github.com/SilverRainZ/srain.git
    $ cd srain

Complile:

.. literalinclude:: build.sh
    :language: shell

Install(root privileges required):

.. literalinclude:: install.sh
    :language: shell

.. note::

    The configure script **doesn't** check any dependience. So you should make
    sure that you have fully installed all dependencies.

Install documents:

.. code-block:: console

    $ make doc
    # make install-doc

Distribution Package
====================

Arch Linux
----------

Packages `srain`_ and `srain-git`_ (git version) are available on AUR,
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

`dfceaef`_ has written `Debian package script for Srain`_, but it is already
**out of date**, I will be glad if anyone can fix it.

.. _dfceaef: https://github.com/yangfl
.. _Debian package script for Srain: https://github.com/SilverRainZ/srain/tree/debian/debian
