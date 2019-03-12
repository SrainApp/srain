====================
Configuration Manual
====================

Srain uses `Libconfig`_ to process configuration file.

.. _Libconfig: http://www.hyperrealm.com/libconfig/

.. contents::
    :local:
    :depth: 3
    :backlinks: none

Configuration File
==================

The path of system wide configuration file ``builtin.cfg`` depends on the
compile flag ``--sysconfdir``, default to be ``/etc/srain``.

The path of user wide configuration file ``srain.cfg`` is ``$XDG_CONFIG_HOME/srain``,
usually it is ``~/.config/srain``.

The difference between system wide and user wide configuration file is the
priority, **The user wide configuration always overwrite the one in system wide
configuration**. For more details about priority, refer to
:ref:`config-priority-and-fallback`.

Syntax
======

For the syntax of configuration file, please refer to Libconfig's documentation:
`Configuration Files`_

.. _Configuration Files: http://www.hyperrealm.com/libconfig/libconfig_manual.html#Configuration-Files

For an example configuration, refer to :ref:`config-example`.

.. _config-priority-and-fallback:

Priority and Fallback
=====================

A **group** is a collection of configurations. The same group in different place
have different priority.

.. code-block:: default

    server: { # A top level ``server`` group

        # ...

        user = {}
        default-messages = {}
        irc = {}
        chat = { } # A ``chat`` group directly in ``server``

        chat-list = (
            { }, # A ``chat`` group in ``chat-list``
            # ...
        )
    }

    server-list: (
        { }, # A ``server`` group in ``server-list``
        # ...
    )

For example, A ``server`` group contains the information(not all) of connection
to IRC servers.  ``server`` group can appear at the top level of configuration
file, or be an element of ``server-list`` list with a unique ``name``. The top
level ``server`` is often used to specify the global configuration, and
``server-list`` is used to specify specified IRC servers.

The configuration in ``server-list`` can overwrite the top level ``server``, and
if an option is not specified in ``server-list``, It will fallback to ``server``.

Another similar group is ``chat``, chat contains configuration of chat panel,
such as whether to show topic, whether to save chat log and etc. Every ``server``
group can contain a ``chat`` group and a ``chat-list`` list. Elements in
``chat-list`` are also identified with an unique ``name``.

As same as the relationship between top level ``server`` and ``server-list``,
``chat-list`` is used to specify a specified chat and has higher priority.

As mentioned above, The priority of user configuration is always higher than
system's, so we can get the priority sorting of ``server`` and ``chat`` group:

.. code-block:: none

    server-list(user) >
    top level server(user) >
    server-list(system) >
    top level server(system)

    chat-list in server-list(user) >
    chat in server-list(user) >
    chat-list in top level server(user) >
    chat in top level server(user) >
    chat-list in server-list(system) >
    chat in server-list(system) >
    chat-list in top level server(system) >
    chat in top level server(system)

.. _config-example:

All Configurable Items
======================

Here is the default system wide configuration file used by Srain, all
configurable items are already listed here. you can make a copy as your user
configuration file, but note:

1. User configuration always overwrite system's, **if you don't know what does
   this option means, please remove it from your user configuration rather than
   overwrite it**
2. The ``server``'s ``name`` in ``server-list`` is unique, please remoeve the
   duplicated ``server`` in ``server-list`` before using

.. literalinclude:: ../data/builtin.cfg
