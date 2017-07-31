===========
Quick Start
===========

After the :doc:`installation` of Srain, you will find Srain in your
applications list, if not, just type ``srain`` in your shell to run it. Then you
will see Srain's interface.

.. figure:: http://img.vim-cn.com/17/43f247f4b152149a09dde488e13cda2e641893.png

.. contents::
    :local:
    :depth: 3
    :backlinks: none

Connect Using Interface
=======================

Click the connection button, enter the address of IRC server address and your
nickname on the connection panel, then click the "connect" button.

.. figure:: http://img.vim-cn.com/e4/c5455bb099b9d5bb8dbea2e0557f39cff768eb.png

Now you have connected to freenode:

.. figure:: http://img.vim-cn.com/95/c1e2e7c3498bb9344455ed863533a8eb2add37.png

Click the join button, enter the name of channel which you want to join:

.. figure:: http://img.vim-cn.com/f1/b3694602ff3017b79bced2325e16e6e0acdfc6.png

Now you have joined the channel `#srain`_.

.. _#srain: ircs://chat.freenode.org:6697/srain

.. figure:: http://img.vim-cn.com/b8/0d8802e02701358ed448e4f052039af67b7b6e.png


Connect Using Command
=====================

Enter the following commands in input entry::

    /server connect freenode
    /join #srain

If you want to automatically execute commands at each time Srain starts, please
refer to :ref:`commands-context`.
