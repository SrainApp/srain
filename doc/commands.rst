===============
Commands Manual
===============

.. contents::
    :local:
    :depth: 3
    :backlinks: none

.. _commands-context:

Context
=======

User can run commands by typing them in the input entry on UI.

The rc(**r**\ un **c**\ ommand) file ``srainrc`` allows you save commands in
file and run it at each time the Srain starts. The location of rc file is
``$XDG_CONFIG_HOME/srain``, usually it is ``~/.config/srain``. Every line in rc
file should be a valid command, empty line or line prefixed with ``#`` will be
ignored.

Here is an example of rc file:

.. literalinclude:: ../srainrc.example

.. _commands-syntax:

Syntax
======

A command is a line of string that has the following format, different elements
are separated by whitespace::

    <name> [subcommand] [<option> [value]]... [argument]...

The command's ``name`` starts with a slash ``/`` and doesn't contain any
whitespace, such as: ``/join``.

A ``subcommand`` is a instruction behind the command name, it is optional.

An ``option``, as its name, is optional too, starts with a hyphen ``-`` and may
has a ``value``.

``option`` is often used as the switch of a flag or a way to change some
default values. For example, The command ``/connect -tls -port 6697``,
option ``-tls`` is just a flag that tells srain use secure connections with TLS.
And ``-port`` requires a ``value``, if ``-port`` is not specified, program will
use the default value ``6667``. Check :ref:`commands-connect` for more details.

If a ``vaule`` starts with a hyphen ``-`` or contains whitespaces, it should be
enclosed by single quotation mark ``'``.

.. note::

    All ``option`` should appear behind ``subcommand`` (If any), and before
    ``argument``.

An ``argument`` is similar to ``vaule``, but commonly it doesn't have a default
value so it can not be omitted (actually it depends on the implement of the
command). If the first ``argument`` starts with a hyphen ``-``, it should be
enclosed by single quotation mark ``'``. If an ``argument`` contains whitespaces,
it should be quoted too. Specially, the last argument can contain whitespace
without quoted.

If you want to use a single quotation mark in a quoted text, use backslash ``\``
to escape it. For backslash itself, use double backslash ``\\``.

Usage
=====

.. _commands-server:

/server
-------

Usage::

    /server [add|rm|list|set|connect|disconnect]
        [-host <host>] [-port <port>] [-pwd <password>] [-tls] [-tls-not-verify]
        [-nick <nickname>] [-user <username>] [-real <realname>] <name>

IRC server management.

Sub commands:

* ``add``: create a server with unique name, you can set server information via
  options
* ``rm``: remove a server, all options will be ignored, you can only remove a
  server which is unconnected
* ``list``: list all available servers
* ``set``: set server information via options
* ``connect``: connect to specified server, all options will be ignored
* ``disconnect``: disconnect from specified server, all options will be ignored

Create a server and connect to connect to it immediately. It will become the
default server automaticly.

Options:

* ``-host``: server host
* ``-port``: server port, default ``6667``
* ``-pwd``: connection password, default empty
* ``-tls``: use secure connections with TLS
* ``-tls-not-verify``: don't verify TLS certificate
* ``-nick``: specify nickname
* ``-user``: specify username, default same as nickname
* ``-real``: specify realname, default same as nickname

Arguments:

* ``name``: unique name of server

.. _commands-connect:

/connect
--------

Usage::

    /connect [-port <port>] [-pwd <password>] [-tls] [-tls-not-verify]
        [-user <username>] [-real <realname>] <host> <nick>

Create a IRC server and connect to it immediately.

Options:

* ``-port``: server port, default ``6667``
* ``-pwd``: connection password, default empty
* ``-tls``: use secure connections with TLS
* ``-tls-not-verify``: don't verify TLS certificate
* ``-user``: specify usernamem default same as nickname
* ``-real``: specify realname, default same as nickname

Arguments:

* ``host``: server host
* ``nick``: specify nickname

Example::

    /connect -real 'I am srainbot' -tls -port 6697 chat.freenode.org srainbot
    /connect 127.0.0.1 srainbot

--------------------------------------------------------------------------------

.. note::

    The following commands should run under the context which has a
    "default server", Briefly, **these command must executed after**
    :ref:`commands-server` ``connect`` **or** :ref:`commands-connect`
    **command.**

.. _commands-relay:

/relay & /unrelay
-----------------

Usage::

    /relay [-cur] <nick>
    /unrelay [-cur] <nick>

Flag ``nick`` as a relay bot, show the real nick of the message sender.
Use ``[`` and ``]`` as delimiter.

Refer :ref:`faq-relay-message-transform` to see its effect.

Options:

* ``-cur``: only effects the current chat

Example::

    /relay teleboto

.. warning::

    This command is unstable, it may be implement as a plugin in the future.

/ignore & /unignore
-------------------

Usage::

    /ignore [-cur] <nick>
    /unignore [-cur] <nick>

Ignore/unignore somebody's message.

Options:

* ``-cur``: only ignore in current chat

.. _commands-rignore:

/rignore & /unrignore
---------------------

Usage::

    /rignore [-cur] <name> <pattern>
    /unignore [-cur] <name>

Ignore/unignore message which matches specified pattern.

Options:

* ``-cur``: only ignore in current chat

Arguments:

* ``name``: unique pattern name
* ``pattern``: perl-compatible regex expression which is used to match the
  incoming message, for regex syntax, refer to
  https://developer.gnome.org/glib/stable/glib-regex-syntax.html

/query & /unquery
-----------------

Usage::

    /query <nick>
    /unquery [nick]

Start/stop private chat with somebody. For ``/unquery`` , If ``nick`` no
specified, stop the current private chat.

.. _commands-join:

/join
-----

Usage::

    /join <channel>[,<channel>]... [<passwd>[,<passwd>]]...

Join specified channel(s), channels are separated by commas ``,``.

Example::

    /join #archinux-cn,#gzlug,#linuxba
    /join #channel1,#channe2 passwd1

/part
-----

Usage::

    /part [<channel>[,<channel>]]... [<reason>]

Leave specified channel(s) with optional reason, channels are separated by
commas ``,``. If ``channel`` no specified, leave the current channel.

Example::

    /part #archinux-cn Zzz...
    /part #archlinux-cn,#tuna
    /part

/quit
-----

Usage::

    /quit [reason]

Quit current server with optional reason.

/topic
------

Usage::

    /topic [-rm|<topic>]

Set the current channel's topic. If ``topic`` no specified, just display the
current channel's topic.

* ``-rm``: remove current channel's topic

Example::

    # Just view the topic
    /topic
    # Set the topic to "Welcome to Srain!"
    /topic Welcome to Srain!
    # Clear the topic
    /topic -rm

/msg
----

Usage::

    /msg <target> <message>

Send message to a target, the target can be channel or somebody's nick. If you
want to send a message to channel, you should :ref:`commands-join` it first.

/me
---

Usage::

    /me <message>

Send a action message to the current target.


/nick
-----

Usage::

    /nick <new_nick>

Change your nickname.

/whois
------

Usage::

    /whois <nick>

Get somebody's information on the server.

/invite
-------

Usage::

    /invite <nick> [channel]

Invite somebody to join a channel. If ``channel`` not specified, fallback to
current channel.

/kick
-----

Usage::

    /kick <nick> [channel] [reason]

Kick somebody from a channel, with optional reason. If ``channel`` not specified,
fallback to current channel.

/mode
-----

Usage::

    /mode <target> <mode>

Change ``target`` 's mode.

/list
-----

Usage::

    /list

List all channels on the default server.

.. warning::

    This command is not implemented yet.
