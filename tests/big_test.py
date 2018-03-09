#!/usr/bin/env python2
import sys
import socket
import traceback
import urllib
import struct

class Test_req:
    d_oflow_ch = "a"
    d_oflow_l = 9001

    def __init__(self, meth="GET", host= "localhost", uri="/",
                 port=":8001", ver="HTTP/1.0", end="\r\n",
                 spot1="", spot2=" ", spot3=" ", spot4=""):
        self.spot1 = spot1
        self.meth = meth
        self.spot2 = spot2
        self.host = host
        self.port = port
        self.uri = uri
        self.spot3 = spot3
        self.ver = ver
        self.spot4 = spot4
        self.end = end
#        self.d = dict(d_test=Test_req.build_test_string(self))

    def make_req_line(self):
        return self.spot1 + self.meth + self.spot2 + self.host + \
            self.port + self.uri + self.spot3 + self.ver + self.spot4 + self.end
    
'''
def make_req_line(d):
    req = ""
    req += d['spot1'] + d['meth'] + d['spot2'] + d['host'] + d['port'] + \
           d['uri'] + d['spot3'] + d['ver'] + d['spot4'] + d['end']
    return req
'''
def send_req(host, port, req):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Connecting to %s:%d..." % (host, port))
    sock.connect((host, port))

    print("Connected, sending request...")
    sock.send(req)

    print("Request sent, waiting for reply...")
    rbuf = sock.recv(1024)
    resp = ""

    while len(rbuf):
        resp = resp + rbuf
        rbuf = sock.recv(1024)

    print("Recieved reply.")
    sock.close()
    return resp

if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " host port")
    exit()

try:
    req_line = Test_req()
    r = ""
    i = 0
    while i<9000:
        i+=1
        r+= "a"
    
#    req_line.uri = r
    test_req = req_line.make_req_line()
    
    print("HTTP request:")
    print(test_req)

    resp = send_req(sys.argv[1], int(sys.argv[2]), test_req)
    print("HTTP response:")
    print(resp)

#    test_str_dict = build_test_string_dic()
#    for test_str in test_strs_default.d

except:
    print("Exception:")
    print(traceback.format_exc())

    
