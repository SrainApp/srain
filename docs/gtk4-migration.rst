==============
GTK4 Migration
==============

This repository is still a GTK3 application. A direct switch to ``gtk4`` would
break large parts of the UI layer because several core widgets and APIs are no
longer available.

Current blockers
================

- ``GtkMenu`` and ``populate-popup`` are used for buffer, nick, and message
  context menus. These need ``GMenuModel`` plus ``GtkPopoverMenu``.
- ``GtkTreeView`` powers the join panel and user list. GTK4 requires
  ``GtkColumnView`` or ``GtkListView`` plus factories.
- ``gtk_container_*``, ``gtk_box_pack_*``, and ``gtk_bin_get_child()`` are
  widely used in ``src/sui/`` and must be replaced with GTK4 child APIs.
- ``gtk_dialog_run()`` is still used for synchronous dialogs and must be
  replaced by signal-driven response handling.

What is in place now
====================

- ``meson_options.txt`` includes ``gtk4_experimental`` as the migration switch.
- ``script/report-gtk4-blockers.sh`` reports the remaining GTK3-only API usage.
- ``src/inc/gtk_compat.h`` centralizes low-risk CSS and icon helpers that work
  in both GTK3 and GTK4.
- The application-level menu in ``src/sui/sui_app.c`` is now backed by
  ``GMenuModel`` instead of Glade-defined ``GtkMenuItem`` trees.
- ``data/ui-gtk4/app_menu.blp`` now captures the GTK4 Blueprint shape for the
  application popover menu and shared menu models.

Recommended order
=================

1. Replace menus and popup flows with action-based popovers.
2. Port ``GtkTreeView`` screens to list/column factories.
3. Remove ``GtkContainer`` and ``GtkBin`` helpers from composite widgets.
4. Convert Glade templates into Blueprint once each widget tree is GTK4-safe.
