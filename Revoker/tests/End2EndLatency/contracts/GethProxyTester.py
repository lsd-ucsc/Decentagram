#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import binascii
import json
import socket


HOST_ADDR = 'localhost'
HOST_PORT = 51234


def DoRequest(sock: socket.socket, request: dict) -> dict:
	requestJson = json.dumps(request)
	print('Sending request: {}'.format(requestJson))

	# send data length
	sock.sendall(len(requestJson).to_bytes(8, byteorder='little'))
	# send data
	sock.sendall(requestJson.encode('utf-8'))

	# receive data length
	dataLen = int.from_bytes(sock.recv(8), byteorder='little')
	# receive data
	data = sock.recv(dataLen)

	print('Received data: {}'.format(data.decode('utf-8')))
	return json.loads(data)


def main() -> None:
	# Oracle publish
	request = {
		'method': 'OraclePublish',
		'params': [
			binascii.hexlify(b'Hello World!').decode('utf-8'),
		]
	}
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect((HOST_ADDR, HOST_PORT))
	DoRequest(sock, request)

	# import time
	# time.sleep(10)

	# SubscriberTransact
	request = {
		'method': 'SubscriberTransact',
		'params': [
			binascii.hexlify(b'Hello World!').decode('utf-8'),
		]
	}
	# sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	# sock.connect((HOST_ADDR, HOST_PORT))
	DoRequest(sock, request)

	# Invalid method
	request = {
		'method': 'InvalidMethod',
		'params': [ ]
	}
	# sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	# sock.connect((HOST_ADDR, HOST_PORT))
	DoRequest(sock, request)


if __name__ == '__main__':
	main()

