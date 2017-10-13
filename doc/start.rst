===========
Quick Start
===========

.. contents::
    :local:
    :depth: 3
    :backlinks: none

.. note::

    If you are using version 0.06.2 and earlier, please refer to:
    http://doc.srain.im/en/0.06.2/start.html .
    Although using earlier version is not recommended.

Start Srain
===========

After the :doc:`installation` of Srain, you will find Srain in your
applications list, if not, just type ``srain`` in your shell to run it. Then you
will see Srain's Initial interface.

.. figure:: _static/srain-startup.png

Here is the layout of Srain being used:

.. figure:: _static/srain-layout.png

Connect to IRC server
=====================

`freenode`_ is a famous IRC network, the official channel of Srain `#srain`_ is
also hosted on it. Let's start by connecting to freenode.

.. _freenode: https://freenode.net/

Connect to Predefined Server
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since version 0.06.3, Srain has a predefined list of commonly used IRC servers,
freenode is one of it. If you want to add server into this list, refer to
:doc:`prefs`.

Click the connection button on the Srain header bar, select the page "Predefined
Server", select the item "freenode" from candidate box with label "Server",
enter your nickname, then click the "connect" button:

.. figure:: _static/srain-connect-predefined-server.png

If everything goes well, srain should connected to freenode, then your would see
some message from freenode's server:

.. figure:: _static/srain-connected-server.png

Connect to Custom Server
~~~~~~~~~~~~~~~~~~~~~~~~

While the server your want to connect to is not listed in the predefined list,
switch to the page "Custom Server", enter the host name, port, and etc.

.. note::

    - If the port is a TLS port, make sure that the option
      "Use secure connection" is checked.
    - If the server's certificate is untrusted and you insist on continuing,
      check the option "Not verify certificate". Then, click the "connect"
      button.

Then click the "connect" button:

.. figure:: _static/srain-connect-custom-server.png

Using command
~~~~~~~~~~~~~

Refer to :ref:`commands-server` and :ref:`commands-connect`.

If you want to automatically execute commands at each time Srain starts, please
refer to :ref:`commands-context`.

Join Channel
============

After connecting to freenode, now let's try to join `#srain`_, the official
channel of Srain.

.. _#srain: ircs://chat.freenode.org:6697/srain

Directly Join
~~~~~~~~~~~~~

While your has a clear channel to join, such as `#srain`_, just click the join
button, select the page "Join Channel", enter the channel name in the input
entry with label "Channel", then click the "join" button:

.. figure:: _static/srain-join-channel.png

Now you should joined the channel:

.. figure:: _static/srain-joined-channel.png

Search Channel
~~~~~~~~~~~~~~

While you don't know the exact name of the channel, click the join button,
switch to page "Search Channel", click the button with a "refresh" icon, Srain
should start receiving channel list from server:

.. figure:: _static/srain-search-channel.png

Then you can enter the keyword or specify the filter conditions to search
channel. For example we enter "sra", select the channel you want to join from
channe list, then click "join" button:

.. figure:: _static/srain-searched-chennel.png

.. note::

    If the channel requires a join password, try double click the row
    of channel.

Using command
~~~~~~~~~~~~~

Refer to :ref:`commands-join`.
