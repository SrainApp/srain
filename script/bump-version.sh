#!/bin/sh

OLDVER=$1
NEWVER=$2

if [ ! -e LICENSE ]; then
    echo "Please run this script in root of project directory" >&2
    exit 1
fi

# Version in Makefile.in
sed -i "s/^PACKAGE_NEWVER = ${OLDVER}/PACKAGE_NEWVER = ${NEWVER}/" "Makefile.in"

# Version in source code preface comment
for c in $(find ./src -name '*.c'); do
    sed -i "s/^ \* @version ${OLDVER}/ * @version ${NEWVER}/" ${c}
done
for css in $(find ./data/themes -name '*.css'); do
    sed -i "s/^ \* @version ${OLDVER}/ * @version ${NEWVER}/" ${css}
done

# Version in configuration file
sed -i "s/version = \"${OLDVER}\"/version = \"${NEWVER}\"/" "data/builtin.cfg"

# Version in desktop file
sed -i "s/Version=${OLDVER}/Version=${NEWVER}/" "data/Srain.desktop"

# Version in documentations
sed -i "s/version = '${OLDVER}'/version = '${NEWVER}'/" "doc/conf.py"
sed -i "s/release = '${OLDVER}'/release = '${NEWVER}'/" "doc/conf.py"
for rst in $(find ./doc -name '*.rst'); do
    sed -i "s/${OLDVER}\.tar\.gz/${NEWVER}.tar.gz/" ${rst}
    sed -i "s/srain-${OLDVER}/srain-${NEWVER}/" ${rst}
done

echo Please check carefully after bumping version!
