#!/bin/bash
set -ex
# Silence all safe.directory errors:
#   fatal: detected dubious ownership in repository at '/__w/srain/srain'
git config --global --add safe.directory '*';
# Fetch all tags.
git fetch origin +refs/tags/*:refs/tags/*;
git tag;
# Basic environment variables.
SRAIN_HOME=$PWD;
SRAIN_TAG=`git rev-list --tags --max-count=1`;
SRAIN_TAG_NAME=`git describe --tags $SRAIN_TAG`;
SRAIN_TAG_COMMITTER_NAME=`git log $SRAIN_TAG -n 1 --pretty=format:"%an"`;
SRAIN_TAG_COMMITTER_EMAIL=`git log $SRAIN_TAG -n 1 --pretty=format:"%ae"`;
SRAIN_TAG_DATE=`git log $SRAIN_TAG -n 1 --pretty=format:"%ad" --date=format:'%a, %d %b %Y %H:%M:%S %z'`;
# Install the dependencies:
# Debian building packages: debhelper, dpkg-dev
# Make dependencies: gettext, libconfig-dev, libgtk-3-dev, libsecret-1-dev, libsoup2.4-dev, libssl-dev, pkg-config
# Runtime dependencies: glib-networking, libgtk-3-0, libsecret-1-0, libconfig9, libsoup2.4
# Python3 script: python3 python3-requests
apt-get install -y debhelper dpkg-dev gettext libconfig-dev libgtk-3-dev libsecret-1-dev libsoup2.4-dev libssl-dev pkg-config glib-networking libgtk-3-0 libsecret-1-0 libconfig9 libsoup2.4 python3 python3-requests meson libayatana-appindicator3-dev;
# Download the debian files.
git clone https://github.com/SrainApp/srain-contrib.git --depth 1;
cd srain-contrib;
mv pack/debian $SRAIN_HOME/debian;
cat > $SRAIN_HOME/debian/changelog << EOF
srain ($SRAIN_TAG_NAME) unstable; urgency=medium

  * New upstream version $SRAIN_TAG_NAME

 -- $SRAIN_TAG_COMMITTER_NAME <$SRAIN_TAG_COMMITTER_EMAIL>  $SRAIN_TAG_DATE
EOF
cd $SRAIN_HOME;
rm -rf srain-contrib;
# Build the package.
dpkg-buildpackage -b -us -uc;
mkdir $SRAIN_HOME/out;
mv $SRAIN_HOME/../srain_"$SRAIN_TAG_NAME"_amd64.deb $SRAIN_HOME/out/
# Basic test.
apt-get install -f $SRAIN_HOME/out/srain_"$SRAIN_TAG_NAME"_amd64.deb;
/usr/bin/srain --version;
