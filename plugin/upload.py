# upimg plugin example
#
# upimg accept a file path and return a url

import requests

url = 'http://img.vim-cn.com'

def upload(img):
    with open(img,'rb') as f:
        res = requests.post(url, files = {'name': f})
        return res.text
