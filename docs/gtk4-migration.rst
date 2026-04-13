==============
GTK4 Migration
==============

The default local build now targets ``gtk4``. Most composite GTK4 templates
have also been mirrored into ``data/ui-gtk4/*.blp`` so Blueprint can serve as
the editable source format while ``.ui`` files remain the checked-in generated
artifacts.

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
- ``script/update-gtk4-blueprints.sh`` regenerates checked-in ``data/ui-gtk4/*.ui``
  files from ``.blp`` sources. ``make blueprints`` wraps this workflow.
- ``src/inc/gtk_compat.h`` centralizes low-risk CSS and icon helpers that work
  in both GTK3 and GTK4.
- The application-level menu in ``src/sui/sui_app.c`` is now backed by
  ``GMenuModel`` instead of Glade-defined ``GtkMenuItem`` trees.
- ``data/ui-gtk4/*.blp`` now covers the GTK4 window, buffer, connect panel,
  join panel, user list, side bar item, image window, and application menu
  templates.
- ``buffer_menu.glade`` has been removed; buffer and chat menu items are now
  created in code, which isolates the remaining menu migration to runtime APIs.
- ``nick_menu.glade`` has been removed; nick context menus are now built in
  code, leaving fewer UI resources tied to deprecated GTK menu widgets.

Recommended order
=================

1. Replace menus and popup flows with action-based popovers.
2. Port ``GtkTreeView`` screens to list/column factories.
3. Remove ``GtkContainer`` and ``GtkBin`` helpers from composite widgets.
4. Remove leftover Glade resources and stop checking in generated GTK4 ``.ui``
   files once the project is ready to rely on Blueprint at build time only.
