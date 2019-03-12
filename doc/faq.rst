==========================
Frequently Asked Questions
==========================

.. contents::
    :local:
    :depth: 3
    :backlinks: none

Does Srain support Windows?
===========================

Yes, please refer to :ref:`install-packages-windows`.

Why does the image upload button not work?
==========================================

This function is implemented by a python plugin, therefore you should install
``python3-urllib3`` and ``python3-request`` to let it work.

.. warning:: Image upload function is unavailable after :ref:`version-1.0.0rc1`.

Why can't I see people's avatar?
================================

There is not a specification for user avatar in IRC protocol
(`IRCv3`_ has an idea about it), so currently the avatar function is simply
implemented by a python plugin, therefore you should install
``python3-urllib3`` and ``python3-request`` to let it work.

Besides, you should set ``show_avatar`` to ``true`` in your configuration.

.. _IRCv3: http://ircv3.net/

.. warning:: Avatar function is unavailable after :ref:`version-1.0.0rc1`.

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
