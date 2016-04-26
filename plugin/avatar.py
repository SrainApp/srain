##
# @file avatar.py
# @brief avatar plugin example
# @author LastAvengers <lastavengers@outlook.com>
# @version 1.0
# @date 2016-03-15

import urllib.request as ur
import json

def test():
    if avatar('iovxw', '', '') == 'https://avatars.githubusercontent.com/iovxw?s=36':
        print('test1 passed')
        if avatar('LastAvengers', '', '') == 'https://s.gravatar.com/avatar/f122d4b03bb7c1e7fd3d89a92a6f2ea9?s=36':
            print('test2 passed')
            return

    print('failed')

api = 'https://avatars.githubusercontent.com/{{USERNAME}}?s=36'

def avatar(nick, user, host):
    # with open('./archives.json') as f:
        # archives = json.loads(f.read())
    # if nick in archives:
        # return archives[nick]
    # else:
    return api.replace('{{USERNAME}}', nick)

if __name__  == '__main__':
    test()
