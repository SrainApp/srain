==========
Change Log
==========

.. contents::
    :local:
    :depth: 1
    :backlinks: none

2018-02-28 Version 0.06.4
=========================

- Changed:

  - Change default application ID to ``im.srain.Srain``

- Added:

  - CTCP support, including request & response CLIENTINFO, FINGER, PING,
    SOURCE, TIME, VERSION, USERINFO messages. DCC message is **not** yet
    supported. Use command :ref:`commands-ctcp` for sending a CTCP request
  - Login method support, you can specify it by configuration file option
    ``server.login_method``:

      - ``sasl_plain``: SASL PLAIN authentication support, will use
        ``server.user.username`` as identity, and use ``server.user.passwd`` as
        password

  - Added documentation :doc:`support` used to show Srain's features,
    inspried by https://ircv3.net/software/clients.html
  - Added a semantic version parser, not yet used
  - Added appdata file which requier by application store, thanks to @cpba
  - openSUSE package is available, please refer to
    :ref:`install-packages-opensuse` for details, thanks to @alois
  - Flatpak package is available, please refer to
    :ref:`install-packages-flatpak` for details, thanks to @cpba

- Improved:

  - Fixed a logical error in IRC message parser: all parameters are equal
    whether matched by ``<middle>`` or ``<trailing>``, thanks to @DanielOaks
  - Improved connection state control, you can smoothly disconnect/quit from
    server even it is unresponsive
  - Fixed truncated message output by :ref:`commands-server` ``list``
    subcommand
  - Fixed crash at ``g_type_check_instance()`` under GLib 2.54.3+
  - Fixed: Do not free a SrianServerBuffer which has non-empty buffer
  - Ensure the QUIT message can be sent before application shutdown
  - Removed entry from desktop file, thanks to @TingPing
  - Fixed grammer of join message, thanks to @raindev
  - Re-enable CI for Srain: |ci-status|

.. |ci-status| image:: https://travis-ci.org/SrainApp/srain.svg?branch=master
    :target: https://travis-ci.org/SrainApp/srain

2017-12-22 Version 0.06.3
=========================

- Changed:

  - Configurable file option ``tls_not_verify`` in ``irc`` block in ``server``
    block is renamed to ``tls_noverify``, old option name is still supported
  - Command option ``tls-not-verify`` for :ref:`commands-server` and
    :ref:`commands-connect` is renamed to ``tls-noverify``, old option name
    is still supported

- Added:

  - Connect popover supports connect to predefined server
  - Join popover supports channel search

- Improved:

  - Modified margin and padding of some widgets
  - Improved the style of unread message counter
  - Fixed markup parse error of decorator
  - Fixed crashing while connecting from connect popover
  - Fixed use after free while removing user
  - Improved the performance and extensibility of user list
  - Improved compatibility with older versions of GTK(> 3.16)
  - Refactor the code of chat panel, helpful for the next development

2017-09-12 Version 0.06.2
=========================

- Added:

  - mIRC color support, can be disabled via setting ``render_mirc_color``
    option in ``chat`` block in ``server`` block to ``false``

- Improved:

  - Better error reporting while operating the UI
  - IRC URL can be opened directly within the application
  - Text in input entry, connection panel and join panel will not be cleared
    while operation is not successful
  - Fixed: in some cases, nickname registration will case infinity loop
  - Decorator and filter now can process xml message
  - Imporved the handling of channel topic

2017-08-18 Version 0.06.1
=========================

- Added:

  - Added GPL copyright statements
  - ``RPL_CHANNEL_URL`` (328) message support
  - Command line options support, type ``srain -h`` for help message
  - Support for Creating server and joining channel from IRC URL
  - New dependency libsoup
  - Add reconnect timer: if connection fails, Srain will wait for 5 seconds
    then try to connect again. If it still fails, waiting time will increase by
    5 second

- Improved:

  - Fixed the crash when QUIT
  - Fixed: avoid sending empty password
  - More empty parameters checks
  - Imporve server connection status control

2017-07-29 Version 0.06
=======================

- Changed:

  - The third time of refactor ;-)
  - New command parser, for the syntax, refer to :ref:`commands-syntax`.
  - Changed the format of Chat log
  - The :ref:`commands-relay` command doesn't support custom delimiter, this function will
    be implemented as python plugin in the future
  - Use reStructuredText for document instead of Markdown

- Added:

  - Message filter: mechanism for filtering message
  - Message Decorator: mechanism for changing message
  - Install script for Gentoo, thanks to @rtlanceroad !
  - New command :ref:`commands-rignore` for ignore message using regular
    expression, thanks to @zwindl !
  - Config file support
  - Configurable log module, more convenient for developing and reporting issue
  - New Return value type, for more friendly error reporting
  - New command :ref:`commands-server` for IRC servers management
  - Srain home page is available at: https://srain.im
  - Srain help documentation is available at: https://doc.srain.im

- Removed:

  - Remove libircclient dependence

- Improved:

  - Improve reconnection stuff: auto reconnect when ping time out
  - More accurate message mention
  - Display preview image in correct size
  - Any number of image links in message can be previewed
  - HTTP(and some other protocols) link, domain name, email address and IRC
    channel name in topic and messages can be rendered as hyper link
  - The sent message can be merged to last sent message
  - Fixed some bugs

2016-09-19 Version 0.05.1
=========================

- Create missing directory: ``$XDG_CACHE_HOME/srain/avatars``

2016-08-27 Version 0.05
=======================

- Changed:

  - Port to libircclient

    - SSL connection support
    - Server password support
    - Channel password support

- Added:

  - Convenience GtkPopover for connecting and joining
  - Nick popmenu
  - Translations: zh_CN
  - Forward message
  - Chat log
  - Colorful user list icon
  - Mentioned highlight
  - Desktop notification

- Improved:

  - More friendly User interface
  - Stronger {upload,avatar} plugin
  - Fixed a lot of bugs

.. note::

    0.05 is the first stable release of Srain, enjoy~

2016-04-30 Version 0.03
=======================

- New interface between UI and IRC module
- Multi-server support
- Channel name is not case sensitive now
- /quit command will close all SrainChan of a server
- Fix GTK-Warning when close a SrainCHan

.. note::

    0.03 is a pre-release, some functions are no completed yet.
    it also has some undetected bugs.

2016-04-13 Version 0.02
=======================

- Bugs fixed
- Port to GTK+-3.20

.. note::

    0.02 is a pre-release, some functions are no completed yet.
    it also has some undetected bugs.

2016-04-07 Version 0.01
=======================

- Implement basic functions of a IRC client
- Themes: Silver Rain (light)
- Simple python plugin support:

  - Auto upload image to pastebin (img.vim-cn.org)
  - Get github avatar according nickname
  - NB: plugin will separated from this repo in the future

- Image preview from URL
- Relay bot message transfrom
- Nick auto completion
- Combine message from same person

.. note::

    0.01 is a pre-release, some functions are no completed yet.
    it also has some undetected bugs.
