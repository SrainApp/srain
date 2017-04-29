=========================
Change Log for Srain User
=========================

This change log is oriented to Srain's user, mainly records changes in the usage
of Srain. For changes in architecture, design and code, refer to commit logs of
Srain's git repositories.

2017-05-xx Version 0.06 [draft]
===============================

- Changed:

  - The third time of refactor ;-)
  - New command parser, for the syntax, refer to ``./doc/commands.rst``
  - Changed the format of Chat log
  - The ``/relay`` command doesn't support custom delimiter, this function will
    be implemented as python plugin in the future
  - Use reStructuredText for document instead of Markdown

- Added:

  - Message filter: mechanism for filtering message
  - Message Decorator: mechanism for changing message
  - Install script for Gentoo, thanks to @rtlanceroad !
  - Support to ignore message using  regular expression, thanks to @zwindl !
    Use command ``/rignore <name> <pattern>`` to have a try.
  - Config file support [WIP]

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

0.05 is the first stable release of Srain, enjoy~

2016-04-30 Version 0.03
=======================

- New interface between UI and IRC module
- Multi-server support
- Channel name is not case sensitive now
- /quit command will close all SrainChan of a server
- Fix GTK-Warning when close a SrainCHan

NB: 0.03 is a pre-release, some functions are no completed yet.
it also has some undetected bugs.

2016-04-13 Version 0.02
=======================

- Bugs fixed
- Port to GTK+-3.20

NB: 0.02 is a pre-release, some functions are no completed yet.
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

NB: 0.01 is a pre-release, some functions are no completed yet.
it also has some undetected bugs.
