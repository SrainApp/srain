==========
Change Log
==========

.. contents::
    :local:
    :depth: 1
    :backlinks: none

.. Please write changelog as the following template:

    .. _version-x.x.x:

    YYYY-MM-DD Version X.X.X
    ========================

   .. post:: YYYY-MM-DD
      :category: release

    - Features:

      - XXX (:pull:`PULL_REQUEST_ID`)
      - XXX (:issue:`ISSUE_ID`)
      - XXX (:commit:`COMMID_ID`)

    - Changes:

      - XXX

    - Bug fixes:

      - XXX

.. _version-latest:

.. _version-1.4.0:

2022-05-07 Version 1.4.0
========================

.. post:: 2020-05-07
   :category: release

- Features:

  - Add support for IRCv3 standard-replies (:pull:`354`), by @progval
  - Add support for SASL EXTERNAL (:pull:`352`), by @progval
  - Add support for invite-notify (:pull:`359`), by @progval
  - Add conf item ``server-visibility`` for setting default server visiblity (:issue:`361`), by @SilverRainZ
  - Add support for server-time (:pull:`345`), by @progval
  - Add :ref:`commands-clear` command for clearing messages of current buffer (:issue:`268`), by @SilverRainZ

- Bug fixes:

  - Fix workflow for building MS Windows release artifact (:issue:`360`), by @lifeibiren

- Changes:

  - Make the buffer menu apply to the right-clicked item instead of the active buffer (:pull:`353`), by progval
  - Srain now accepting donations through OpenCollective (https://opencollective.com/srain)
  - Disable workflow for building debian release artifact since we have offical debian package (:commit:`99d9d86`)

.. _version-1.3.2:

2022-02-10 Version 1.3.2
========================

.. post:: 2022-02-10
   :category: release

- Features:

  - Add French translation (:pull:`351`), by @progval

- Bug fixes:

  - Silence assertion failure on /part command (:pull:`348`), by @progval
  - Silence "Unknown capability" warning on trailing whitespace (:pull:`349`), by @progval

- Changes:

  - Add tooltips to buttons with no text(:pull:`350`), by @progval
  - Srain is added to offical repository of Debian, see :ref:`install-packages-debian` for details

.. _version-1.3.1:

2021-12-18 Version 1.3.1
========================

.. post:: 2021-12-18
   :category: release

- Bug fixes:

  - Fix build of macOS (:pull:`339`), by @SilverRainZ
  - Make channel messages with origin out of channels can be shown (:pull:`336`), by @progval
  - Rejoin channels after NICKSERV authentication finishes (:issue:`274`), by @SilverRainZ

.. _version-1.2.5:

2021-12-18 Version 1.2.5
========================

.. post:: 2021-12-18
   :category: release

- Bug fixes:

  - Backport :pull:`336`, :issue:`274`

.. _version-1.3:

2021-09-23 Version 1.3.0
========================

.. post:: 2021-09-23
   :category: release

- Features:

  - Implement ISUPPORT parsing + the UTF8ONLY IRCv3 specification (:pull:`331`), thanks to @progval
  - Add support for RPL_UMODEIS (:pull:`331`), thanks to @progval
  - Make nick completion case-insensitive (:pull:`333`), thanks to @progval
  - Add button for inserting emoji (:issue:`279`)

- Bug fixes:

  - Fix configuration syntax errors caused by trailing commas (:pull:`330`), thanks to @progval

.. _version-1.2.4:

2021-07-18 Version 1.2.4
========================

.. post:: 2021-07-18
   :category: release

- Bug fixes:

  - Fix bug casued by :pull:`316` (:issue:`319`)
  - Fix crach when URL preview async task failed (:issue:`322`)

.. _version-1.2.3:

2021-06-26 Version 1.2.3
========================

.. post:: 2021-06-26
   :category: release

- Bug fixes:

  - Eliminate GDK warning (:pull:`316`)

.. _version-1.2.2:

2021-05-30 Version 1.2.2
========================

.. post:: 2021-05-30
   :category: release

- Changed:

  - Add libera.chat IRC network (:pull:`311`)
  - Updated Dutch translation (:commit:`b6830e9`)

- Bug fixes:

  -  Fix windows build (:pull:`300`, :pull:`303`)

.. _version-1.2.1:

2021-04-02 Version 1.2.1
========================

.. post:: 2021-04-02
   :category: release

- Features:

  - Add Ukrainian translations (:pull:`292`), thanks to :people:`andmizyk`

- Bug fixes:

  -  Minor fixes (:issue:`290`)

.. _version-1.2.0:

2021-02-28 Version 1.2.0
========================

.. post:: 2021-02-28
   :category: release

- Features:

  - Switch build system from Make to Meson (:pull:`266`)
  - Add FreeBSD implementations for ``srn_get_executable_{path,dir}``,
    thanks to :people:`wahjava`
  - Add ``/quote`` command for sending special IRC commands,
    thanks to :people:`hno` (:pull:`283`)
  - Add support for hiding server buffer (:pull:`287`)


- Bug fixes:

   - Fix an use-after-free BUG (:pull:`267`)
   - Fix implicit declaration error on some systems,
     thanks to :people:`lgbaldoni` (:pull:`270`)

.. _version-1.1.3:

2020-10-01 Version 1.1.3
========================

.. post:: 2020-10-01
   :category: release

- Bug fixes:

   - Fix an use-after-free BUG (:pull:`267`)

.. _version-1.1.2:

2020-08-10 Version 1.1.2
========================

.. post:: 2020-08-10
   :category: release

.. note::

    This release contains only improvement for MS Windows,
    user of other platform can ignore it.

- Features:

  - Binary for MS Windows now can automatically built via Github Actions,
    thanks to :people:`lifeibiren` (:pull:`259`), please refer to
    :ref:`install-packages-windows` for more details

- Changes:

  - For ease of running on windows, Srain's executable path is added to the
    search paths of {configuration,data} file. Thanks to :people:`lifeibiren`
    (:pull:`259`)

.. _version-1.1.1:

2020-06-27 Version 1.1.1
========================

.. post:: 2020-06-27
   :category: release

- Changes:

  - Improve auto-scroll policy of message list (:pull:`255`)

- Bug fixes:

  - Fix TLS certificate verification error on glib-networking 2.64.3 (:issue:`251`)
  - Fix crash when connecting to an invalid host (:issue:`234`)

.. _version-1.1.0:

2020-05-24 Version 1.1.0
========================

.. post:: 2020-05-24
   :category: release

- Features:

  - Support multiple selection of message (:issue:`138`)
  - Support jump to mentioned message (:pull:`243`)
  - Nickname will be highlighted when mentioned (:pull:`243`)
  - Auto build deb package (:pull:`238`)

- Changes:

  - Improve fcous control of UI
  - Replace appdata with metainfo (:pull:`240`)
  - Validate metainfo with appstream-util (:issue:`242`)

- Bug fixes:

  - Some implicit declarations fixes (:pull:`236`)
  - Some typo fixes (:pull:`239`)

.. _version-1.0.2:

2020-04-11 Version 1.0.2
========================

.. post:: 2020-04-11
   :category: release

- Features:

  - Add MAN documentation (:commit:`deaf723`)
  - Add more predefined IRC networks: DALnet, EFnet, IRCnet, Undernet and QuakeNet (:pull:`228`)

- Changes:

  - Build: Allow setting CC variable via environment (:pull:`224`)
  - Add channel related messages to corresponding buffer as possible (:issue:`149`)
  - Improve widget focus control (:pull:`229`)
  - Drop unused icons (:commit:`6239fe5`)
  - Provide clearer error message when connecting (:pull:`233`)
  - Update gentoo installation documentation :ref:`install-packages-gentoo` (:commit:`ceb5ca3`)

- Bug fixes:

  - Truncate long message before sendisg (:pull:`227`)
  - Deal with invalid UTF-8 string (:commit:`50e7757`)
  - Fix incorrect user number of channel user list (:pull:`230`)
  - Fix incorrect icon install path (:commit:`9f07380`)

.. _version-1.0.1:

2020-03-14 Version 1.0.1
========================

.. post:: 2020-03-14
   :category: release

- Features:

  - Auto rename to original nick when ghost quit (:pull:`198`)
  - Add hackint IRC network (:pull:`201`), thanks to :people:`kpcyrd`
  - Add Dutch translation (:pull:`215`), thanks to :people:`Vistaus`
  - Add two FAQs to documentation (:pull:`217`)
  - Add debian pack script (:contrib-pull:`1`), thanks to :people:`tomac4t`.
    Please refer to :ref:`install-packages-debian` to build a deb package

- Changes:

  - Move continuous integration from travis CI to github actions
    (:pull:`203`, :pull:`204`), thanks to :people:`tomac4t`
  - Make header bar buttons repect default belief (:pull:`205`, :pull:`218`)

- Bug fixes:

  - Fix case sensitivity issue for IRC messages (:pull:`202`),
    thanks to :people:`hhirtz`
  - Fix invalid changelog section of appdata file (:pull:`214`)
  - Fix missing dependences in documentation (:pull:`216`),
    thanks to :people:`avoidr`

.. _version-1.0.0:

2020-02-24 Version 1.0.0
========================

.. post:: 2020-02-24
   :category: release

- Changes:

  - Some code cleanup
  - Update :doc:`./start` documentation

- Bug fixes:

  - Allow Srain runs without dbus secrets service (:issue:`195`)
  - Fix nick generation logical (:commit:`39ced08`)

.. note::

    1.0.0 is the first stable release of Srain, enjoy!

.. _version-1.0.0rc9999:

2019-10-07 Version 1.0.0rc9999
==============================

.. post:: 2019-10-07
   :category: release

- Features:

  - Activate corresponding buffer when channel URL is clicked (:pull:`190`)
  - Command alias support (:issue:`188`)
  - List predefined servers via command, see :ref:`commands-server` for details
    (:commit:`656f3e5`)

- Changes:

  - Replace all icons with freedesktop standard icons (:issue:`120`)

- Bug fixes:

  - Fix wrong usage of GError (:issue:`179`)
  - Fix image preview problem when image is hard to detect type (:issue:`163`)
  - Fix memory leak of pattern filter (:commit:`9464a9e`)
  - Fix the breaking "abort sending" icon (:pull:`144`)
  - Fix the invite menu of user (:commit:`9f98cbb`)

.. _version-1.0.0rc5:

2019-06-14 Version 1.0.0rc5
===========================

.. post:: 2019-06-14
   :category: release

- Added:

   - Regular expression pattern management using :ref:`commands-pattern` command
   - Add command :ref:`commands-filter` for filtering message via pattern
   - Add command :ref:`commands-render` for rendering message via pattern

- Changed:

   - Refactor detector module and rename it to render
   - Refactor filter module
   - Change project description

- Removed:

   - Drop command :ref:`commands-rignore`
   - Drop command :ref:`commands-relay`

.. _version-1.0.0rc4:

2019-05-13 Version 1.0.0rc4
===========================

.. post:: 2019-05-13
   :category: release

- Added:

   - New dependency ``libsecret``
   - Add password storage support
   - Add command line options ``--no-auto``, used to require Srain not to
     automatically connect to servers
   - Add russian translation, thanks to @tim77
   - Allow user send slash(``/``) prefixed message

- Removed:

  - Drop all password fields in configuration file

- Changed:

  - Enable CSD(Client-Side Decoration) by default
  - Update chinese translation

.. _version-1.0.0rc3:

2019-04-14 Version 1.0.0rc3
===========================

.. post:: 2019-04-14
   :category: release

.. _version-1.0.0rc2:

2019-01-24 Version 1.0.0rc2
===========================

.. post:: 2019-01-24
   :category: release
.. _version-1.0.0rc1:

2018-09-10 Version 1.0.0rc1
===========================

.. post:: 2018-09-10
   :category: release

.. _version-0.06.4:

2018-02-28 Version 0.06.4
=========================

.. post:: 2018-02-28
   :category: release

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

.. post:: 2017-12-22
   :category: release

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

.. _version-0.06.2:

2017-09-12 Version 0.06.2
=========================

.. post:: 2017-09-12
   :category: release

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

.. post:: 2017-08-18
   :category: release

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

.. post:: 2017-07-29
   :category: release

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
  - Srain home page is available at: https://srain.im [expired]
  - Srain help documentation is available at: https://doc.srain.im [expired]

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

.. post:: 2016-09-19
   :category: release

- Create missing directory: ``$XDG_CACHE_HOME/srain/avatars``

2016-08-27 Version 0.05
=======================

.. post:: 2016-08-27
   :category: release

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

2016-04-30 Version 0.03
=======================

.. post:: 2016-04-30
   :category: release

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

.. post:: 2016-04-13
   :category: release

- Bugs fixed
- Port to GTK+-3.20

.. note::

    0.02 is a pre-release, some functions are no completed yet.
    it also has some undetected bugs.

2016-04-07 Version 0.01
=======================

.. post:: 2016-04-07
   :category: release

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
