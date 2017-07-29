==========================
Frequently Asked Questions
==========================

.. contents::
    :local:
    :depth: 3
    :backlinks: none

Is Srain support Windows?
=========================

No yet, Windows support is in plan.

Why does the upload image button not works?
===========================================

This function is implemented by a python plugin, you should install
``python3-urllib3`` and ``python3-request`` to let it works.

Why can't I see people's avatar?
================================

There is not a specification for user avatar in IRC protocol
(`IRCv3`_ has an idea about it), so currently the avatar function is simply
implemented by a python plugin, you should install ``python3-urllib3`` and
``python3-request`` to let it works.

Beside that, you should set ``show_avatar`` to ``true`` in your configuration.

.. _IRCv3: http://ircv3.net/

.. _faq-relay-message-transform:

What is "relay message transform"?
==================================

There are many relay bots forward messages from other IM to IRC network,
"Relay message transform" make these messages easier to read.

For example:

.. image:: http://img.vim-cn.com/7f/5211f94b8bcfabf16a852907bc76001ee321be.png

To enable "relay message transform", check :ref:`commands-relay` for more
details.

Where are the log files?
========================

Refer to :ref:`misc-chat-logs`.
