#!/usr/bin/env python3

import subprocess as sp
import os
from pprint import pprint

os.environ.clear()

cgi_env = {
'QUERY_STRING': '',
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
# os.environ['QUERY_STRING'] = '127.0.0.1&52000'
os.environ['QUERY_STRING'] = 'h1=127.0.0.1&p1=52000&f1=t1.txt'
os.environ['QUERY_STRING'] += '&h2=127.0.0.1&p2=52001&f2=t2.txt'
os.environ['QUERY_STRING'] += '&h3=127.0.0.1&p3=52002&f3=t3.txt'
os.environ['QUERY_STRING'] += '&h4=127.0.0.1&p4=52003&f4=t3.txt'
# os.environ['QUERY_STRING'] += '&h5=127.0.0.1&p5=52004&f5=t5.txt'

# pprint(dict(os.environ))

sp.call(['request_server.cgi'])

