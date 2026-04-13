==============
GTK4 Migration
==============

The default local build now targets ``gtk4``. Most composite GTK4 templates
have also been mirrored into ``data/ui-gtk4/*.blp`` so Blueprint can serve as
the editable source format while ``.ui`` files remain the checked-in generated
artifacts.

Current status
==============

- The default local build, install, and startup path now work on ``gtk4``.
- ``script/report-gtk4-blockers.sh`` reports zero hits for the original
  GTK3-only API blocker set.
- The remaining migration work is no longer centered on widget availability;
  it is mostly about long-tail GTK3 fallback resources, keeping checked-in
  ``.ui`` files synchronized with Blueprint sources, and broader runtime
  regression coverage.

What is in place now
====================

- ``meson_options.txt`` includes ``gtk4_experimental`` as the migration switch.
- ``script/report-gtk4-blockers.sh`` reports the remaining GTK3-only API usage.
- ``script/update-gtk4-blueprints.sh`` regenerates checked-in ``data/ui-gtk4/*.ui``
  files from ``.blp`` sources. ``make blueprints`` wraps this workflow.
- ``script/check-gtk4-blueprints.sh`` verifies that checked-in GTK4 ``.ui``
  files still match the generated Blueprint output. ``make blueprints-check``
  wraps this verification.
- ``src/inc/gtk_compat.h`` centralizes low-risk CSS and icon helpers that work
  in both GTK3 and GTK4.
- The application-level menu in ``src/sui/sui_app.c`` is now backed by
  ``GMenuModel`` instead of Glade-defined ``GtkMenuItem`` trees.
- ``data/ui-gtk4/*.blp`` now covers the GTK4 window, buffer, connect panel,
  join panel, user list, side bar item, image window, and application menu
  templates.
- ``src/sui/`` still keeps GTK3 fallback Glade paths for widgets such as
  ``window``, ``buffer``, ``connect_panel``, ``join_panel``, ``user_list``,
  ``side_bar_item``, ``message_list``, ``recv_message``, ``misc_message``,
  ``send_message``, ``url_previewer``, and ``prefs_dialog``.
- ``buffer_menu.glade`` has been removed; buffer and chat menu items are now
  created in code, which isolates the remaining menu migration to runtime APIs.
- ``nick_menu.glade`` has been removed; nick context menus are now built in
  code, leaving fewer UI resources tied to deprecated GTK menu widgets.

Recommended order
=================

1. Keep expanding runtime validation for the GTK4 path, especially around
   window, buffer, connect, and join flows.
2. Remove leftover GTK3-only Glade resources once their fallback paths are no
   longer needed.
3. Decide whether checked-in GTK4 ``.ui`` files remain a permanent artifact or
   become generated-only build outputs.
