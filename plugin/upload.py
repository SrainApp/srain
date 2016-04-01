##
# @file upload.py
# @brief upload plugin example
# @author LastAvengers <lastavengers@outlook.com>
# @version 1.0
# @date 2016-03-15
#
# upload a file to img.vim-cn.com
# function upload() accept a file path and return a url
#

import requests

url = 'http://img.vim-cn.com'

def test():
    img = '../data/img/default_avatar.png'
    res = upload(img)
    if res == 'http://img.vim-cn.com/2c/bb501d2f4e918f77f8dcb67c11cfeefd07627f.png':
        print("test passed")
    else:
        print("test failed")

def upload(img):
    with open(img,'rb') as f:
        res = requests.post(url, files = {'name': f})
        if (res.text.startswith('http')):
            return res.text.strip('\n')
        else:
            return NULL

if __name__ == '__main__':
    test()
