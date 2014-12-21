#!/usr/bin/env python3

import subprocess as sp
import os
from pprint import pprint

os.environ.clear()

cgi_env = {
'QUERY_STRING': 'h1=127.0.0.1&p1=52000&f1=t1.txt',
'CONTENT_LENGTH': '',
'REQUEST_METHOD': 'GET',
'SCRIPT_NAME': '',
'REMOTE_HOST': 'localhost',
'REMOTE_ADDR': '127.0.0.1',
'REMOTE_USER': '',
'REMOTE_IDENT': '',
'AUTH_TYPE': '',
}

for k, v in cgi_env.items():
    os.environ[k] = v

# pprint(dict(os.environ))

sp.call(['./cgi_test.py'])

