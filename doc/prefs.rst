==================
Preferences Manual
==================

Srain uses `Libconfig`_ as configuration file backend.

.. _Libconfig: http://www.hyperrealm.com/libconfig/

.. contents::
    :local:
    :depth: 3
    :backlinks: none

Configuration File Location
===========================

The path of sysetm wide configuration file ``builtin.cfg`` depends on the
compile flag ``--config-dir``, default to be ``/etc/srain``.

The path user wide configuration file ``srain.cfg`` is ``$XDG_CONFIG_HOME/srain``,
usually it is ``~/.config/srain``.

The difference between sysetm wide and user wide configuration file is the
priority, **The configuration in user wide configuration always overwrite the
one in system wide configuration file**. For more details about priority,
refer to :ref:`prefs-priority-and-fallback`.

Syntax
======

For the syntax of configuration file, please refer to Libconfig's documentation:
`Configuration Files`_

.. _Configuration Files: http://www.hyperrealm.com/libconfig/libconfig_manual.html#Configuration-Files

.. _prefs-priority-and-fallback:

Priority and Fallback
=====================

.. code-block:: nginx

    log: {}
    application: {}
    server: {

        # ...

        user = {}
        default_messages = {}
        irc = {}
        chat = { };

        chat_list = (
            { }, # A ``chat`` group
            # ...
        );
    };

    server_list: (
        { }, # A ``server`` group
        # ...
    );

All Configurable Items
======================

Here is a default system wide configuration file, all configurable items are
listed here.

.. literalinclude:: ../data/builtin.cfg
    :language: nginx
