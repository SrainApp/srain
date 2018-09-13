#!/bin/sh

case "${PLATFORM}" in
    debian)
        cat > /etc/apt/apt.conf <<EOF
APT::Get::Assume-Yes "true";
APT::Get::force-yes "true";
APT::Get::Install-Recommends "false";
APT::Get::Install-Suggests "false";
EOF
        apt update -q
        apt-get install make gcc pkg-config gettext libgtk-3-dev libsoup2.4-dev libconfig-dev
        apt-get install glib-networking libgtk3.0 libsoup2.4 libconfig9
        ;;
    *)
        echo "Unsupported platfrom ${PLATFORM}"
        exit 1
        ;;
esac
