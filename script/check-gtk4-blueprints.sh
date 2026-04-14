#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
COMPILER=${BLUEPRINT_COMPILER:-blueprint-compiler}
TMPDIR=$(mktemp -d)
STATUS=0
EXPECTED_MANUAL_UIS="misc_message
prefs_dialog
recv_message
send_message"

cleanup() {
    rm -rf "$TMPDIR"
}

trap cleanup EXIT INT TERM

cd "$REPO_ROOT"

"$COMPILER" batch-compile \
    "$TMPDIR" \
    "$REPO_ROOT" \
    "$REPO_ROOT/data/ui-gtk4/"*.blp

for blueprint in "$REPO_ROOT"/data/ui-gtk4/*.blp; do
    base=$(basename "$blueprint" .blp)
    generated="$TMPDIR/data/ui-gtk4/$base.ui"
    checked_in="$REPO_ROOT/data/ui-gtk4/$base.ui"

    if ! cmp -s "$generated" "$checked_in"; then
        diff -u "$checked_in" "$generated" || true
        STATUS=1
    fi
done

cd "$REPO_ROOT/data/ui-gtk4"
ls *.ui | sed 's/\.ui$//g' | sort > "$TMPDIR/all-ui.txt"
ls *.blp | sed 's/\.blp$//g' | sort > "$TMPDIR/all-blp.txt"
cd "$REPO_ROOT"

actual_manual_uis=$(comm -23 "$TMPDIR/all-ui.txt" "$TMPDIR/all-blp.txt")

if [ "$actual_manual_uis" != "$EXPECTED_MANUAL_UIS" ]; then
    printf '%s\n' "Unexpected manual GTK4 UI files:" >&2
    printf '%s\n' "$actual_manual_uis" >&2
    STATUS=1
fi

exit "$STATUS"
