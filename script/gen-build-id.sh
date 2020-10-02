#!/bin/sh
#
# Usage: ./bump-version.sh <project root directory>
#

# Check argument
[ -z $1 ] && echo Missing target directory && exit 1


# Whether a git repo
cd $1 && [ -d .git ] && git rev-parse --git-dir > /dev/null 2>&1 || exit 0

# Generate build ID
echo -n -git@0.`git rev-list --count HEAD`.`git describe --always`
