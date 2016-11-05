#!/usr/bin/env sh

if [ "$1" = "--gen" ]; then
    cmd="{$(grep "\"\/" ./srv/srv_session_cmd.c | grep -o '".*"' | tr '\n' ',' | sed 's/",/ ",/g')0}"
    echo "Commands supported by srain: ${cmd}"
    sed -i "s|{0}|${cmd}|g" ./inc/cmd_list.h
elif [ "$1" = "--restore" ]; then
    sed -i "s|{.*}|{0}|g" ./inc/cmd_list.h
fi
