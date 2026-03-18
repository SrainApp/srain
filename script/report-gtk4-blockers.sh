#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

patterns=(
  "GtkMenu\\b"
  "GtkTreeView\\b"
  "\\bgtk_container_add\\s*\\("
  "\\bgtk_container_remove\\s*\\("
  "\\bgtk_container_get_children\\s*\\("
  "\\bgtk_container_foreach\\s*\\("
  "\\bgtk_box_pack_start\\s*\\("
  "\\bgtk_box_pack_end\\s*\\("
  "\\bgtk_bin_get_child\\s*\\("
  "\\bgtk_dialog_run\\s*\\("
  "\\bgtk_menu_shell_append\\s*\\("
  "\\bgtk_menu_popup_at_pointer\\s*\\("
  "\\bgtk_menu_button_set_popup\\s*\\("
  "\\bgtk_event_box_new\\s*\\("
)

for pattern in "${patterns[@]}"; do
  count="$(rg -n "$pattern" src data -g '!builddir/**' || true)"
  count="$(printf '%s\n' "$count" | sed '/^$/d' | wc -l)"
  printf '%-32s %s\n' "$pattern" "$count"
done
