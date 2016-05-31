#!/usr/bin/sh

if [[ "$1" == "--gen" ]]; then
    cmd="{$(grep "IS_CMD" ./server/server_cmd.c | grep -o '".*"' | tr '\n' ',')0}"
    sed -i "s|{0}|${cmd}|g" ./inc/cmd_list.h
elif [[ "$1" == "--restore" ]]; then
    sed -i "s|{.*}|{0}|g" ./inc/cmd_list.h
fi
