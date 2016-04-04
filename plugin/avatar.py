##
# @file avatar.py
# @brief avatar plugin example
# @author LastAvengers <lastavengers@outlook.com>
# @version 1.0
# @date 2016-03-15

import urllib.request as ur

api = 'https://avatars.githubusercontent.com/{{USERNAME}}?s=36'

def avatar(nick, user, host):
    return api.replace('{{USERNAME}}', nick)
