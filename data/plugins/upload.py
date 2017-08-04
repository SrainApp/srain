#!/usr/bin/env python3
##
# @file upload.py
# @brief upload plugin example
# @author Shengyu Zhang <srain@srain.im>
# @version 1.0
# @date 2016-03-15
#
# Upload a file to img.vim-cn.com
# upload() accept a file path and return a URL or None
#

import requests
from urllib.parse import quote

url = 'http://img.vim-cn.com'
timeout = 30

def test():
    img = '../pixmaps/srain-avatar.png'
    res = upload(img)
    if res == 'http://img.vim-cn.com/2c/bb501d2f4e918f77f8dcb67c11cfeefd07627f.png':
        print("test passed")
    else:
        print("test failed")

def upload(img):
    with open(img,'rb') as f:
        res = requests.post(url,files = {'name': (quote(img), f)}, timeout = timeout)
        if (res.text.startswith('http')):
            return res.text.strip('\n')
        else:
            return None

    return None

if __name__ == '__main__':
    test()
