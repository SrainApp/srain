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

.. _faq-relay-message-transform:

What is "relay message transform"?
==================================

There are many relay bots forward messages from other IM to IRC network,
"Relay message transform" make these messages easier to read.

For example, there is a telegram bot named "telegram", the words in brackets
is the named of the telegram user.

.. figure:: _static/srain-render-message-before.png

Run command ``/pattern add normal-relay \[(?<sender>[^:]+?)\] (?<content>.*)``
and ``/render telegram normal-relay``, you get:

.. figure:: _static/srain-render-message-after.png

For more details, please refer to `commands-pattern` and `commands-render`.

Where are the log files?
========================

Refer to :ref:`misc-chat-logs`.

How can I send message which has a slash("/") prefixed?
=======================================================

Please prepend another slash to the message.

Refer to :ref:`commands-syntax`.

How can I remove my stored password?
====================================

Just leave the password entry empty and check the "Remember password" checkbox
before connect to server or join channel, the corresponding password will be
removed.

Or you can use `secret-tool` (provided by libsecret) to manage all your stored
passwords.
