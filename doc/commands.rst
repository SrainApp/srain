===============
Commands Manual
===============

.. contents::
    :local:
    :depth: 3
    :backlinks: none

.. _commands-playground:

Playground
==========

User can run commands in two ways, the one way is typing them into the input
entry of chat buffer.

The another way is writing the commands to the ``server.auto-run`` and
``chat.auto-run`` configuration items. For more details, please refer to
:doc:`config`.

.. _commands-syntax:

Syntax
======

A command is a line of string that has the following format, different elements
are separated by whitespace::

    <name> [subcommand] [<option> [value]]... [argument]...

The command's ``name`` starts with a slash ``/`` and doesn't contain any
whitespace, such as: ``/join``.

.. note::

   When you want to send a message starts with a slash, please prepend
   another slash to the message.
   For example, you want to send "/this is a slash prefixed message", you need
   to type "//this is a slash prefixed message".

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

.. _commands-usage-context:

/context
--------

.. warning:: This command is **deprecated**.

Usage::

    /context <server> [chat]

Change the current context.

Arguments:

* ``server``: The server you want to operate after
* ``chat``: Optional, the chat you want operate after, if not set, fallback to
  server's current chat

.. _commands-usage-reload:

/reload
-------

Usage::

    /reload

Reload user configuration.

.. _commands-server:

/server
-------

Usage::

    /server [connect|disconnect] <name>

Predefined IRC server management.

.. note:: This command may changes context.

Sub commands:

* ``connect``: connect to specified predefined server
* ``disconnect``: disconnect from specified predefined server

Arguments:

* ``name``: unique name of server

.. _commands-connect:

/connect
--------

Usage::

    /connect [-port <port>] [-pwd <password>] [-tls] [-tls-noverify]
        [-user <username>] [-real <realname>] <host> <nick>

Create a IRC server and connect to it immediately.

.. note:: This command may changes context.

Options:

* ``-port``: server port, default ``6667``
* ``-pwd``: connection password, default empty
* ``-tls``: use secure connections with TLS
* ``-tls-noverify``: use TLS connection without certificate verification
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

    The following commands should run under the specified **context**.
    The context can be changed by :ref:`commands-usage-context`,
    :ref:`commands-server` or :ref:`commands-connect`.

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

    This command is unstable, it may be implemented as a plugin in the future.


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

    /rignore [-cur] <pattern>
    /unrignore [-cur] <pattern>

Ignore/unignore message whose content matches specified pattern.

Pattern can be add via command :ref:`commands-pattern`.

Options:

* ``-cur``: only ignore in current chat

Arguments:

* ``pattern``: name of regular expression pattern, use ``/pattern list`` to
  get available name

Example:

This ignore message that content is "Why GTK and not Qt?"::

    /pattern add troll ^Why GTK and not Qt\?$
    /rignore troll

To cancel the ignore of these kind of message, use::

    /unrignore troll

/query & /unquery
-----------------
Usage::

    /query <nick>
    /unquery [nick]

Start/stop private chat with somebody. For ``/unquery`` , If no ``nick`` is
specified, it stops the current private chat.

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
commas ``,``. If no ``channel`` is specified, it leaves the current channel.

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

Set the current channel's topic. If no ``topic`` specified, it just displays
the current channel's topic.

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

Send an action message to the current target.


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

Invite somebody to join a channel. If no ``channel`` is specified, it falls
back to current channel.

/kick
-----

Usage::

    /kick <nick> [channel] [reason]

Kick somebody from a channel, with optional reason. If no ``channel`` is
specified, it falls back to current channel.

/mode
-----

Usage::

    /mode <target> <mode>

Change ``target`` 's mode.

.. _commands-ctcp:

/ctcp
-----

Usage::

    /ctcp <target> <command> [message]

Send a CTCP request to ``target``. The commonly used commands are:
CLIENTINFO, FINGER, PING, SOURCE, TIME, VERSION, USERINFO. For the detail of
each command, please refer to https://modern.ircdocs.horse/ctcp.html .

If you send a CTCP PING request without ``message``, you will get the latency
between the ``target``.

.. note::

    DCC is not yet supported.

.. _commands-pattern:

/pattern
--------

Usage::

    /pattern add <name> <pattern>
    /pattern rm <name>
    /pattern list

Regular expression pattern management.
The added pattern can be used elsewhere in the application, such as
:ref:`commands-rignore`.

Sub commands:

* ``add``: add a pattern with given name
* ``rm``: remove a pattern with given name
* ``list``: list all availables patterns

Arguments:

* ``name``: unique name of pattern
* ``pattern``: a valid `Perl-compatible Regular Expression`_

.. _Perl-compatible Regular Expression: https://developer.gnome.org/glib/stable/glib-regex-syntax.html
