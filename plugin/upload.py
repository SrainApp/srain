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

def upload(img):
    with open(img,'rb') as f:
        res = requests.post(url, files = {'name': f})
        return res.text
