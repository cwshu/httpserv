#!/usr/bin/python3

import cgi
import socket
form = cgi.FieldStorage()

for i in form.keys():                                
    print("{k}: {v}".format(k=i, v=form.getvalue(i)))

hosts = ['h1', 'h2', 'h3', 'h4', 'h5']
connect_hosts = []

for idx, host_key in enumerate(hosts):
    if host_key in form:
        port_key = 'p' + host_key[1]
        file_key = 'f' + host_key[1]
        host = form.getvalue(host_key)
        port = form.getvalue(port_key)
        batch_file = form.getvalue(file_key)
        connection = {
            'host': host,
            'port': port,
            'batch_file': batch_file,
        }
        connect_hosts.append(connection)

for host in connect_hosts:
    print(host)
