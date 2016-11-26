# xxxx-xx-xx Version 0.06:

- Changed:
    - New command parser and format, see `./doc/commands.rst`
    - Scroll message list via key Ctrl + UP/DONW but not UP/DOWN
    - Install script for gentoo, thx to @rtlanceroad !
- Added:
    - Switch history input message via key UP/DOWN
- Improved:
    - More accurate message mention
    - Display preview image in correct size
    - SrainSysMsg:
        - Image in SrainSysMsg can be displayed now
        - HTTP link in SrainSendMsg can be rendered now
    - Mulit sent message can be merged in one SrainSendMsg
    - Define interface signature as macro

# 2016-08-27 Version 0.05:

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

# 2016-04-30 Version 0.03:

- New interface between UI and IRC module
- Multi-server support
- Channel name is not case sensitive now
- /quit command will close all SrainChan of a server
- Fix GTK-Warning when close a SrainCHan

NB: 0.03 is a pre-release, some functions are no completed yet.
it also has some undetected bugs.

# 2016-04-13 Version 0.02:

- Bugs fixed
- Port to GTK+-3.20

NB: 0.02 is a pre-release, some functions are no completed yet.
it also has some undetected bugs.

# 2016-04-07 Version 0.01:

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
