#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
COMPILER=${BLUEPRINT_COMPILER:-blueprint-compiler}

cd "$REPO_ROOT"

"$COMPILER" batch-compile \
    "$REPO_ROOT" \
    "$REPO_ROOT" \
    "$REPO_ROOT/data/ui-gtk4/"*.blp
