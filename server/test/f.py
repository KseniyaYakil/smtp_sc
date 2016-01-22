#!/usr/bin/python

import sys
import json
from os import listdir
from os.path import isfile, join
from socket import create_connection
import socket
import time

def colored_prefix(color, text):
	return '\033[1;%dm%s\033[0m' % (color, text)

def FAIL(*args):
	print "[%s]:" % colored_prefix(31, "FAIL"), " ".join([str(arg) for arg in args])

def OK(*args):
	print "[%s]:" % colored_prefix(32, "OK"), " ".join([str(arg) for arg in args])

def make_test(filename):
	fp = open(filename)
	try:
		cases = json.load(fp)
	except ValueError, msg:
		FAIL("malformed test-case file", filename, "error:", msg)
		return
	try:
		sock = create_connection(("127.0.0.1", 1551))
		failed = False
		for step in cases:
			for key in step:
				print "[client]:", key.replace("\n","\\n").replace("\r", "\\r")
				if len(key):
					sock.send(key)
				if len(step[key]):
					data = sock.recv(1024)
					if not data.startswith(step[key]):
						FAIL("<%s>" % filename, "command:", key, "received:", data, "but expected:", step[key])
						failed = True
						break;
				print "[server]:", data.replace("\n","\\n").replace("\r", "\\r")
			if failed: break;
		if not failed:
			OK(filename)
	except socket.error, msg:
		FAIL("socket error:", msg)
	sock.close()
	fp.close()

test_cmd_files = sys.argv[1:]

for filename in test_cmd_files:
	make_test(filename)
