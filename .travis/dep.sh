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
        ;;
esac

cd /build
./doc/install/${PLATFORM}.sh
