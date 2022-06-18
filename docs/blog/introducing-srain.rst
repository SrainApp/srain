=================
Introducing Srain
=================

.. post:: 2020-02-29
   :category: Announcement

Hi all, I am excited to introduce you to Srain: A modern IRC client written in GTK.

Srain is my personal project since 2016. I released its first stable version recently.
This project originated when I was new to IRC. At that time, I was active
on #archlinux-cn@freenode, learned how to live with Arch Linux.
There are many relay bots in the channel (from XMPP, telegram, Tox and etc).
At first I felt the channel was full of chaos:


* Messages are usually separated into multiple lines which is inconvenient for reading (512 bytes limit of IRC message)
* You have to click the URL and switch to browser to see what it is (titlebot alleviates this problem)
* Messages sent by relay bot have various formats which make it hard to read

In fact, none of thes are problems for experienced IRC users, but in another way,
The inconveniences of IRC actually exists, improving the experience of IRC
chatting is always good.

-- So I wrote a client, and now introduce to you:

.. figure:: /_images/introducing-srain/cover.png

   :ghrepo:`SrainApp/srain`

.. contents::
   :local:

Free and Cross-Platform
-----------------------

Srain is fully open source under GPLv3 license.
You can get software and `source code`__ for free.
Srain supports both Linux, Windows, BSD and macOS, please checkout
:doc:`/install` for more details.

.. hint::

   For Linux user, Srain is available on offical repository of
   flathub__, fedora__ and openSUSE__.

__ https://github.com/SrainApp/srain.im
__ https://flathub.org/apps/details/im.srain.Srain
__ https://src.fedoraproject.org/rpms/srain
__ https://software.opensuse.org/package/Srain

Modern UI
---------

Srain provides a modern graphical user interface which contains:

* Easy to read message bubble
* Convenient popovers for join server and channel
* Optional `Client-side Decoration`__
* HiDPI support
* Custom themes support (see below)

.. figure:: /_images/introducing-srain/main.png

   Main UI

__ https://wiki.gnome.org/Initiatives/CSD

Themes
------

Srain's theme is built on the top of GTK CSS system, so users can easily
customize the theme by writing CSS.

By default, Srain provides two `builtin themes`__: ``default-bubble`` and ``default-bubble-dark`` which works well with most popular GTK themes.

.. figure:: /_images/introducing-srain/theme.png

   Themes

__ https://github.com/SrainApp/srain/tree/master/data/themes

Features
--------

URL Preview
^^^^^^^^^^^

Srain is able to parse the URL in IRC message, detect its content type and show you a preview.

For now, only HTTP URL which points to image can be previewed.

.. hint::

   For convenience, the URL preview function is enabled by default.
   Note that it may **leaks your client IP**, you can disable it in
   :doc:`/config`.


Command System
^^^^^^^^^^^^^^

Like most IRC clients, Srain provides a command system.
Most operations can be done through both GUI and commands.
Please refers to :doc:`/commands` for more details.

Message Filter and Renderer
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Srain provdes regex based message render and filter mechanisms.

Message filter allows ignore one's message by regular expression match.
(:ref:`Ignore by nickname <commands-ignore>` is supported of course).
Please refers to :ref:`commands-filter`.

Message renderer allows user modify one's message by regular expression group capture.
Please refers to :ref:`commands-render`.

**One of the application of message renderer is "relay message transform"**.

There are many relay bots forward messages from other IM to IRC network,
"relay message transform" makes these messages easier to read.
For example, there is a telegram bot named "telegram", the words in brackets
is the named of the telegram user.

.. figure:: /_images/introducing-srain/render-message-before.png

   Before render

Run command ``/pattern add normal-relay \[(?<sender>[^:]+?)\] (?<content>.*)``
and ``/render telegram normal-relay``.
``\[(?<sender>[^:]+?)\] (?<content>.*)`` is a regular expression, which has two
named group ``sender`` and ``content``, when an IRC message matched by this expression:

* The text in ``sender`` group will become the sender of rendered messsage,
* The text in ``content`` group will become the content of rendered messsage.
* The orginal message sender will become the remark of rendered messsage.

The meaning of first command is creating a pattern(regular expression) named
"normal-relay", the second command applies this pattern to the bot "telegram":
all message sent by this bot will be rendered by the aboved pattern.

So, after running the commands, the message from "telegram" looks like the
following figure:

.. figure:: /_images/introducing-srain/render-message-after.png

   After render

Next development
----------------

The two main aims of next development are: use experience improvement and plugin system.

Use Experience Improvement
^^^^^^^^^^^^^^^^^^^^^^^^^^

In most cases, Srain has better user experience than previous clients, but
still has some shortcomings -- which will be improved the future.

The improvement of use experience is strongly based on user's feedback.
If you have any suggestion, please :issue:`file an issue <new>`
or `disscuss in channel #srain`__

__ ircs://chat.freenode.net/#srain

Plugin System
^^^^^^^^^^^^^

Traditional IRC protocol is too simple to meet the needs of modern IM users.
So client should do more, plugin system is a good idea to provide extra functions.

As Srain is a GTK application, I plan to use Libpeas__
to construct Srain's plugin system and provide at least tow kind of plugin:
auto-pastebin(paste your file/image and return URL to your input entry) and
message renderer(render IRC message as as you wish).

You can trace the progress of plugin system at {% include issue id=199 %}.

__ https://wiki.gnome.org/Projects/Libpeas

Energy and Time
^^^^^^^^^^^^^^^

In fact, I am quite busy with both my work and my life, so I can only develop
Srain in my free time. The progress of development may not be too fast,
but it will continue.  If anyone wants to get involved, feel free to contcat me,
you can find contact details on `my homepage`__.

__ https://silverrainz.me/
