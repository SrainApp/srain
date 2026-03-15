#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

patterns=(
  "GtkMenu\\b"
  "GtkTreeView\\b"
  "gtk_container_add"
  "gtk_container_remove"
  "gtk_container_get_children"
  "gtk_container_foreach"
  "gtk_box_pack_start"
  "gtk_box_pack_end"
  "gtk_bin_get_child"
  "gtk_dialog_run"
  "gtk_menu_shell_append"
  "gtk_menu_popup_at_pointer"
  "gtk_menu_button_set_popup"
  "gtk_event_box_new"
)

for pattern in "${patterns[@]}"; do
  count="$(rg -n "$pattern" src data -g '!builddir/**' | wc -l)"
  printf '%-32s %s\n' "$pattern" "$count"
done
