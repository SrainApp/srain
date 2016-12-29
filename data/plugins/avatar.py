##
# @file avatar.py
# @brief Smart avatar plugin for srain
# @author SY Zhang <silverrainz@outlook.com>
# @version 1.0
# @date 2016-08-26
#
# Download a image(avatar) according to `token`, and save it to
# ``~/.cache/srain/avatar/`nick```
#
# token match order:
#   - Custom avatar mapping
#   - "github:<GITHUB ID>"
#   - "gravatar:<MAIL@xxx.com>"
#   - Unknown:
#       - Identicon (暂缓)
#

import os
import shutil
import hashlib
import requests
import threading
from urllib.parse import urlencode

size = 36
timeout = 20

def test():
    pass

def save(url, path):
    r = requests.get(url, timeout = timeout, stream = True)
    if r.status_code == 200:
        with open(path, 'wb') as img:
            shutil.copyfileobj(r.raw, img)
        # print('[save]: %s' % path)
        return True
    else:
        # print('[save]: Failed')
        return False

# ref: https://developer.github.com/v3/users/#get-a-single-user 
def get_github_avatar(id_):
    api = 'https://api.github.com/users/'
    r = requests.get(api + id_, timeout = timeout)
    j = r.json()
    url = j.get('avatar_url')
    if (url):
        url += '&' + urlencode({'s': str(size)})
    # print('[get_github_avatar]: url: %s' % url)
    return url

# ref: https://en.gravatar.com/site/implement/images/python/
def get_gravatar_avatar(email):
    api = 'https://www.gravatar.com/avatar/' + hashlib.md5(email.lower().encode('utf-8')).hexdigest() + '?'
    url = api + urlencode({'s': str(size)})
    # print('[get_gravatar_avatar]: url: %s' % url)
    return url

def get_custom_map_avatar(name):
    api = 'https://raw.githubusercontent.com/SilverRainZ/srain-avatar-archives/master/archives.json'
    r = requests.get(api, timeout = timeout)
    json = r.json()
    url = json.get(name)
    # print('[get_custom_map_avatar]: url: %s' % url)
    return url

def avatar(nick, token, path):
    # print('[avatar]: nick: %s, token: %s, path: %s' % (nick, token, path))
    url = get_custom_map_avatar(nick)

    if not url:
        if token.startswith('github:'):
            url = get_github_avatar(token[len('github:'):])
        elif token.startswith('gravatar:'):
            url = get_gravatar_avatar(token[len('gravatar:'):])

    if not url:
        url = get_github_avatar(nick)

    if url:
        return save(url, path + '/' + nick)

    return False


if __name__  == '__main__':
    test()
