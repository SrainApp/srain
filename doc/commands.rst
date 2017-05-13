========
Commands
========

Command::

    /connect [-port <port>] [-ssl on|off|notverify] [-realname <realname>] [-passwd <passwd>] <host> <nick>

Add a server into your server list. It will become the default server
automaticly.

* ``host``: IRC server host
* ``nick``: The nickname you want to use
* ``-port``: IRC server port, if no specified, use ``6667``
* ``-ssl``:

  - ``on``: Use secure server connections with SSL
  - ``off``: On the contrary
  - ``notverify``: Disables the certificate verification

* ``-passwd``: The password of the server
* ``-realname``: Set your realname

Example::

    /connect -realname 'I am srainbot' -ssl notverify -port 6697 chat.freenode.org srainbot
    /connect 127.0.0.1 srainbot

**The following commands should excuted after a ``/connect`` command**

Command::

    /relay [-cur] <nick>
    /unrelay [-cur] <nick>

Flag ``nick`` as a relay bot, show the real nick of the message sender.
Use ``[`` and ``]`` as delimiter.

Example::

    /relay teleboto

* ``-cur``: Only effects the current chat

Command::

    /ignore [-cur] <nick>
    /unignore [-cur] <nick>

Ignore/unignore somebody's message.

* ``-cur``: Only ignore in current chat

Command::

    /query <nick>
    /unquery [<nick>]

Start/stop private chat with somebody.

Command::

    /join <channel>[,<channel>] [<passwd>[,<passwd>]]

Join specifie channel(s), channels are separated by commas.

Example::

    /join #archinux-cn,#gzlug,#linuxba
    # TODO
    /join #channel1,#channe2 passwd1

Command::

    /part [<channel>[,<channel>]] [<reason>]

Leave specified channel(s) with optional reason, channels are separated by
commas.  If ``channel`` no specified, leave the current channel.

Example::

    /part #archinux-cn Zzz...
    /part #archlinux-cn,#tuna

Command::

    /quit [<reason>]

Quit current server with optional reason.

Command::

    /topic [<topic>|-rm]

Set the current channel's topic. If ``topic`` no specified, just display the
current channel's topic.

* ``-rm``: Remove current channel's topic

Example::

    # Just view the topic
    /topic
    # Set the topic to "Welcome to Srain!"
    /topic Welcome to Srain!
    # Clear the topic
    /topic -rm

Command::

    /msg <target> <message>

Send message to a target, the target can be channel or somebody's nick. If you
want to send a message to channel, you should join it first.

Command::

    /me <message>

Send a action message to the current target.

Command::

    /nick <new_nick>

Change you nick.

Command::

    /whois [<nick>]

Get somebody's information on the server.

Command::

    /invite <nick> [<channel>]

Invite somebody to join a channel. If ``channel`` no specified, fallback to
current channel.

Command::

    /kick <nick> [<channel>] [<reason>]

Kick somebody from a channel, with optional reason. If ``channel`` no specified, fallback to
current channel.

Command::

    /mode <target> <mode>

Change ``target``'s mode.

Command::

    /list

List all channels on the default server.

Note:
    This command is not implemented yet.
