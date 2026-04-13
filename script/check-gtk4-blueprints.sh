#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
COMPILER=${BLUEPRINT_COMPILER:-blueprint-compiler}
TMPDIR=$(mktemp -d)
STATUS=0

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

exit "$STATUS"
