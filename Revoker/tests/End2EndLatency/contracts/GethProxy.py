#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import argparse
import json
import logging
import os
import signal
import socket
import socketserver
import subprocess
import sys
import time

from web3 import Web3
from web3.contract.contract import Contract
from typing import Tuple, Union


HOST_ADDR = 'localhost'
HOST_PORT = 51234

USE_GANACHE = False
KEY_FILE_PATH = '/etc/testnet_keys.json'
GETH_ADDR     = '172.17.0.1'
GETH_PORT     = 8548

THIS_DIR  = os.path.dirname(os.path.abspath(__file__))
TARGET_DIR = os.path.abspath(os.path.join(THIS_DIR, '..'))
BUILD_DIR = os.path.join(THIS_DIR, 'build')
REPO_DIR = os.path.abspath(os.path.join(TARGET_DIR, '..', '..'))
PUBSUB_REPO_DIR = os.path.abspath(os.path.join(REPO_DIR, '..', 'pubsub-onchain'))
PUBSUB_BUILD_DIR = os.path.join(PUBSUB_REPO_DIR, 'build')
DEGETH_REPO_DIR = os.path.abspath(os.path.join(REPO_DIR, '..', 'Ethereum'))

LIBS_DIR = os.path.abspath(os.path.join(REPO_DIR, '..', 'libs'))
PYHELPER2_DIR = os.path.abspath(os.path.join(LIBS_DIR, 'PyEthHelper2'))

SUBS_CONTRACT_BASE_PATH = os.path.join(BUILD_DIR, 'HybridSubscriber')
ORAC_CONTRACT_BASE_PATH = os.path.join(BUILD_DIR, 'Oracle')
PUBS_CONTRACT_BASE_PATH = os.path.join(PUBSUB_BUILD_DIR, 'PubSub', 'PubSubService')

DEGETH_CONF_FILE_PATH = os.path.join(DEGETH_REPO_DIR, 'src', 'components_config.json')

CONF_FILE_PATH = os.path.join(TARGET_DIR, 'components_config.json')


sys.path.append(PYHELPER2_DIR)
from PyEthHelper import EthContractHelper, GanacheAccounts


def HighFeeCalculator(
	ethGasPrice: int,
	ethMaxPriorityFee: int,
) -> Tuple[int, int]:
	# print('ethGasPrice: {}'.format(ethGasPrice))
	# print('ethMaxPriorityFee: {}'.format(ethMaxPriorityFee))

	# determine max priority fee
	# priority fee is 2% of base fee
	maxPriorFee = (ethGasPrice * 2) // 100
	# ensure it's higher than w3.eth.max_priority_fee
	maxPriorFee = max(maxPriorFee, ethMaxPriorityFee)

	maxPriorFee = maxPriorFee * 100

	return maxPriorFee, ethGasPrice + maxPriorFee


def LowFeeCalculator(
	ethGasPrice: int,
	ethMaxPriorityFee: int,
) -> Tuple[int, int]:
	# print('ethGasPrice: {}'.format(ethGasPrice))
	# print('ethMaxPriorityFee: {}'.format(ethMaxPriorityFee))

	# determine max priority fee
	# priority fee is 2% of historical fee
	maxPriorFee = (ethMaxPriorityFee * 1) // 100

	return maxPriorFee, ethGasPrice + maxPriorFee


MidFeeCalculator = EthContractHelper.DefaultFeeCalculator


ExpFeeCalculator = MidFeeCalculator


class GethProxy(object):

	@classmethod
	def LoadContracts(cls, w3: Web3) -> Tuple[Contract, Contract]:
		if True: #USE_GANACHE:
			pubsubContract = EthContractHelper.LoadContract(
				w3=w3,
				projConf=(
					PUBS_CONTRACT_BASE_PATH + '.abi',
					PUBS_CONTRACT_BASE_PATH + '.bin',
				),
				contractName='PubSubService',
				release=None, # use locally built contract
				address=None, # deploy new contract
			)
		else:
			pubsubContract = None

		oracleContract = EthContractHelper.LoadContract(
			w3=w3,
			projConf=(
				ORAC_CONTRACT_BASE_PATH + '.abi',
				ORAC_CONTRACT_BASE_PATH + '.bin',
			),
			contractName='Oracle',
			release=None, # use locally built contract
			address=None, # deploy new contract
		)
		subscriberContract = EthContractHelper.LoadContract(
			w3=w3,
			projConf=(
				SUBS_CONTRACT_BASE_PATH + '.abi',
				SUBS_CONTRACT_BASE_PATH + '.bin',
			),
			contractName='HybridSubscriber',
			release=None, # use locally built contract
			address=None, # deploy new contract
		)
		return oracleContract, subscriberContract, pubsubContract

	@classmethod
	def DeployContracts(
		cls,
		w3: Web3,
		privKey: str,
		pubSub: Union[str, Contract],
		oracleContract: Contract,
		subscriberContract: Contract,
	) -> Tuple[Contract, Contract, Contract]:
		if isinstance(pubSub, str):
			pubSubAddr = pubSub
		else:
			deployReceipt = EthContractHelper.DeployContract(
				w3=w3,
				contract=pubSub,
				arguments=[ ],
				privKey=privKey,
				gas=None, # let web3 estimate
				value=0,
				confirmPrompt=False, # don't prompt for confirmation
				feeCalculator=ExpFeeCalculator,
			)
			pubSubAddr = deployReceipt.contractAddress
			pubSub = EthContractHelper.LoadContract(
				w3=w3,
				projConf=(
					PUBS_CONTRACT_BASE_PATH + '.abi',
					PUBS_CONTRACT_BASE_PATH + '.bin',
				),
				contractName='PubSubService',
				release=None, # use locally built contract
				address=pubSubAddr,
			)
			pubSub.deployedBlockNum = deployReceipt.blockNumber
			pubSub.deployedTxHash = deployReceipt.transactionHash

		# deploy and register the oracle contract
		deployReceipt = EthContractHelper.DeployContract(
			w3=w3,
			contract=oracleContract,
			arguments=[ pubSubAddr, ],
			privKey=privKey,
			gas=None, # let web3 estimate
			value=0,
			confirmPrompt=False, # don't prompt for confirmation
			feeCalculator=ExpFeeCalculator,
		)
		oracleContract = EthContractHelper.LoadContract(
			w3=w3,
			projConf=(
				ORAC_CONTRACT_BASE_PATH + '.abi',
				ORAC_CONTRACT_BASE_PATH + '.bin',
			),
			contractName='Oracle',
			release=None, # use locally built contract
			address=deployReceipt.contractAddress,
		)
		oracleContract.deployedBlockNum = deployReceipt.blockNumber
		oracleContract.deployedTxHash = deployReceipt.transactionHash

		# deploy and subscribe the subscriber contract
		deployReceipt = EthContractHelper.DeployContract(
			w3=w3,
			contract=subscriberContract,
			arguments=[ pubSubAddr, oracleContract.address, ],
			privKey=privKey,
			gas=None, # let web3 estimate
			value=w3.to_wei(0.0001, 'ether'),
			confirmPrompt=False, # don't prompt for confirmation
			feeCalculator=ExpFeeCalculator,
		)
		subscriberContract = EthContractHelper.LoadContract(
			w3=w3,
			projConf=(
				SUBS_CONTRACT_BASE_PATH + '.abi',
				SUBS_CONTRACT_BASE_PATH + '.bin',
			),
			contractName='HybridSubscriber',
			release=None, # use locally built contract
			address=deployReceipt.contractAddress,
		)
		subscriberContract.deployedBlockNum = deployReceipt.blockNumber
		subscriberContract.deployedTxHash = deployReceipt.transactionHash

		return oracleContract, subscriberContract, pubSub

	@classmethod
	def GetEventMgrAddr(cls, w3: Web3, contract: Contract) -> str:
		# get the address of the event manager
		eventMgrAddr = contract.functions.m_eventMangerAddr().call()
		return eventMgrAddr

	def __init__(
		self,
		w3: Web3,
		privKey: str,
		pubSubAddr: str,
	) -> None:
		super(GethProxy, self).__init__()

		self.logger = logging.getLogger(__name__ + '.' + self.__class__.__name__)

		self.w3 = w3
		self.privKey = privKey

		(
			self.oracleContract,
			self.subscriberContract,
			self.pubsubContract,
		) = self.LoadContracts(
			w3=self.w3,
		)
		self.logger.info('Contracts loaded')

		_pubsub = pubSubAddr if self.pubsubContract is None else self.pubsubContract
		(
			self.oracleContract,
			self.subscriberContract,
			self.pubsubContract,
		) = self.DeployContracts(
			w3=self.w3,
			privKey=self.privKey,
			pubSub=_pubsub,
			oracleContract=self.oracleContract,
			subscriberContract=self.subscriberContract,
		)
		self.logger.info(
			'Oracle contract deployed at {}'.format(self.oracleContract.address)
		)
		self.logger.info(
			'Subscriber contract deployed at {}'.format(self.subscriberContract.address)
		)

		self.deploymentRec = {
			'oracle': {
				'blockNum': self.oracleContract.deployedBlockNum,
				'txHash': self.oracleContract.deployedTxHash.hex(),
			},
			'subscriber': {
				'blockNum': self.subscriberContract.deployedBlockNum,
				'txHash': self.subscriberContract.deployedTxHash.hex(),
			},
		}
		if not isinstance(self.pubsubContract, str):
			self.deploymentRec['pubsub'] = {
				'blockNum': self.pubsubContract.deployedBlockNum,
				'txHash': self.pubsubContract.deployedTxHash.hex(),
			}

		self.transactionRec = []

		_eventMgrAddr = self.GetEventMgrAddr(
			w3=self.w3,
			contract=self.oracleContract,
		)
		self.logger.info(
			'Event manager address: {}'.format(_eventMgrAddr)
		)

	def PublishData(self, data: bytes) -> None:
		receipt = EthContractHelper.CallContractFunc(
			w3=self.w3,
			contract=self.oracleContract,
			funcName='onDataAvailable',
			arguments=[ data, ],
			privKey=self.privKey,
			confirmPrompt=False, # don't prompt for confirmation
			gas=None,
			feeCalculator=ExpFeeCalculator,
		)
		self.transactionRec.append({
			'blockNum': receipt.blockNumber,
			'txHash': receipt.transactionHash.hex(),
		})

	def TransactData(self, data: bytes) -> None:
		receipt = EthContractHelper.CallContractFunc(
			w3=self.w3,
			contract=self.subscriberContract,
			funcName='onTransaction',
			arguments=[ data, ],
			privKey=self.privKey,
			confirmPrompt=False, # don't prompt for confirmation
			gas=None,
			feeCalculator=ExpFeeCalculator,
		)
		self.transactionRec.append({
			'blockNum': receipt.blockNumber,
			'txHash': receipt.transactionHash.hex(),
		})


def GethProxyTCPHandler(proxyCore: GethProxy) -> type:
	class TCPHandlerTemplate(socketserver.BaseRequestHandler):

		PROXY_CORE = proxyCore

		def OraclePublish(self, dataHex: str) -> dict:
			if dataHex.startswith('0x'):
				dataHex = dataHex[2:]

			data = bytes.fromhex(dataHex)
			self.PROXY_CORE.PublishData(data=data)

			return {
				'status': 'ok',
				'result': [],
			}

		def SubscriberTransact(self, dataHex: str) -> dict:
			if dataHex.startswith('0x'):
				dataHex = dataHex[2:]

			data = bytes.fromhex(dataHex)
			self.PROXY_CORE.TransactData(data=data)

			return {
				'status': 'ok',
				'result': [],
			}

		def UnknownMethod(self, method: str) -> dict:
			return {
				'status': 'error',
				'result': 'Unknown method: {}'.format(method),
			}

		def handleRequest(self, requestJson: dict) -> dict:
			method = requestJson['method']
			params = requestJson['params']
			if method == 'OraclePublish':
				return self.OraclePublish(*params)
			elif method == 'SubscriberTransact':
				return self.SubscriberTransact(*params)
			else:
				return self.UnknownMethod(method)

		def ConnectionClosing(
			self,
			client: tuple,
			e: Union[Exception, None]
		) -> None:
			if e is None:
				self.PROXY_CORE.logger.info(
					'{}: Connection closed by the client'.format(client)
				)
			else:
				self.PROXY_CORE.logger.info(
					'{}: Exception {}. Closing the socket'.format(client, e)
				)
				# self.PROXY_CORE.logger.exception(e)

		def handle(self):
			self.PROXY_CORE.logger.info('{}: New connection'.format(self.client_address))

			sock: socket.socket = self.request
			while True:
				try:
					# recv 64bit data length
					rawSockData = sock.recv(8)
					if len(rawSockData) == 0:
						self.ConnectionClosing(client=self.client_address, e=None)
						return
					dataLen = int.from_bytes(rawSockData, byteorder='little')
					# recv data
					rawSockData = sock.recv(dataLen)
					if len(rawSockData) == 0:
						self.ConnectionClosing(client=self.client_address, e=None)
						return

					requestJson = json.loads(rawSockData)
					responseJson = self.handleRequest(requestJson)
					responseData = json.dumps(responseJson).encode('utf-8')
					self.PROXY_CORE.logger.info('Response: {}'.format(responseData))

					# send 64bit data length
					sock.sendall(len(responseData).to_bytes(8, byteorder='little'))
					# send data
					sock.sendall(responseData)
				except Exception as e:
					self.ConnectionClosing(client=self.client_address, e=e)
					return

	return TCPHandlerTemplate


def StartGanache() -> subprocess.Popen:
	if not USE_GANACHE:
		# we are using the actual Geth Client, so do nothing
		return None

	global GETH_ADDR, GETH_PORT, GANACHE_KEY_PATH
	GETH_ADDR = 'localhost'
	GETH_PORT = 7545
	GANACHE_KEY_PATH = os.path.join(BUILD_DIR, 'ganache_keys.json')

	_NUM_KEYS = 10
	_NET_ID = 1337
	_NODE_PATH = '/snap/bin/node'
	_GANACHE_PATH = '/usr/local/bin/ganache-cli'

	cmd = [
		_NODE_PATH,
		_GANACHE_PATH,
		'-p', str(GETH_PORT),
		'-d',
		'-a', str(_NUM_KEYS),
		'--network-id', str(_NET_ID),
		'--chain.hardfork', 'shanghai',
		'--wallet.accountKeysPath', str(GANACHE_KEY_PATH),
	]
	proc = subprocess.Popen(
		cmd,
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE
	)

	return proc


def SetupPrivKey(w3: Web3, keyPath: str) -> str:
	if USE_GANACHE:
		keyPath = os.path.join(BUILD_DIR, 'ganache_ckeys.json')
		GanacheAccounts.ChecksumGanacheKeysFile(
			keyPath,
			GANACHE_KEY_PATH
		)

	# setup account
	privKey = EthContractHelper.SetupSendingAccount(
		w3=w3,
		account=0, # use account 0
		keyJson=keyPath
	)

	return privKey


def StopGanache(ganacheProc: subprocess.Popen) -> None:
	if not USE_GANACHE:
		# we are using the actual Geth Client, so do nothing
		return

	print('Shutting down ganache (it may take ~15 seconds)...')
	waitEnd = time.time() + 20
	ganacheProc.terminate()
	while ganacheProc.poll() is None:
		try:
			if time.time() > waitEnd:
				print('Force to shut down ganache')
				ganacheProc.kill()
			else:
				print('Still waiting for ganache to shut down...')
				ganacheProc.send_signal(signal.SIGINT)
			ganacheProc.wait(timeout=2)
		except subprocess.TimeoutExpired:
			continue

	logger = logging.getLogger('ganache-cli')
	for line in ganacheProc.stdout:
		logger.debug(line.decode('utf-8', errors='ignore').strip())
	for line in ganacheProc.stderr:
		logger.error(line.decode('utf-8', errors='ignore').strip())
	print('Ganache has been shut down')


def main():
	argParser = argparse.ArgumentParser(
		description='Geth Proxy'
	)
	argParser.add_argument(
		'--geth-addr', '-g',
		type=str, default='http://{}:{}'.format(GETH_ADDR, GETH_PORT), required=False,
		help='Geth Client HTTP API address'
	)
	argParser.add_argument(
		'--key-file', '-k',
		type=str, default=KEY_FILE_PATH, required=False,
		help='Path to the key file'
	)
	args = argParser.parse_args()

	logging.basicConfig(
		level=logging.INFO,
		format='[%(asctime)s](%(levelname)s) %(name)s: %(message)s'
	)
	logger = logging.getLogger(__name__ + '.' + main.__name__)

	# load configure
	with open(CONF_FILE_PATH, 'r') as f:
		conf = json.load(f)
	pubSubAddr = '0x' + conf['PubSub']['PubSubAddr']

	# start Ganache if necessary
	ganacheProc = StartGanache()
	gethProxy = None
	if args.geth_addr is not None:
		gethUrl = args.geth_addr
	else:
		gethUrl = 'http://{}:{}'.format(GETH_ADDR, GETH_PORT)

	try:
		# connect to Geth Client
		logger.info('Connecting to Geth Client at {}'.format(gethUrl))
		w3 = Web3(Web3.HTTPProvider(gethUrl))
		while not w3.is_connected():
			logger.debug('Attempting to connect to Geth Client...')
			time.sleep(1)
		logger.info('Connected to Geth Client')

		# setup account
		privKey = SetupPrivKey(w3=w3, keyPath=args.key_file)

		gethProxy = GethProxy(
			w3=w3,
			privKey=privKey,
			pubSubAddr=pubSubAddr,
		)
		# gethProxy.PublishData(data=b'Hello World!')
		# gethProxy.TransactData(data=b'Hello World!')

		# update the config file of this project
		if not isinstance(gethProxy.pubsubContract, str):
			# save the address of the pubsub contract
			pubSubAddr = gethProxy.pubsubContract.address
			if pubSubAddr.startswith('0x'):
				pubSubAddr = pubSubAddr[2:]
			conf['PubSub']['PubSubAddr'] = pubSubAddr

		oracleAddr = gethProxy.oracleContract.address
		if oracleAddr.startswith('0x'):
			oracleAddr = oracleAddr[2:]
		conf['PubSub']['PublisherAddr'] = oracleAddr

		subAddr = gethProxy.subscriberContract.address
		if subAddr.startswith('0x'):
			subAddr = subAddr[2:]
		conf['PubSub']['SubscriberAddr'] = subAddr

		with open(CONF_FILE_PATH, 'w') as f:
			json.dump(conf, f, indent='\t')

		# update the config file for Ethereum
		if not isinstance(gethProxy.pubsubContract, str):
			with open(DEGETH_CONF_FILE_PATH, 'r') as f:
				ethConf = json.load(f)

			ethConf['PubSub']['PubSubAddr'] = pubSubAddr
			ethConf['PubSub']['StartBlock'] = gethProxy.pubsubContract.deployedBlockNum

			with open(DEGETH_CONF_FILE_PATH, 'w') as f:
				json.dump(ethConf, f, indent='\t')

		logger.info('Contract deployment done!')
		logger.info('Geth Proxy starts to listen to incoming requests...')
		# start TCP server
		handlerCls = GethProxyTCPHandler(gethProxy)
		with socketserver.TCPServer((HOST_ADDR, HOST_PORT), handlerCls) as server:
			server.serve_forever()

	finally:
		# finish and exit
		StopGanache(ganacheProc)

		if gethProxy is not None:
			# write transaction record
			rec = {
				'deployments': gethProxy.deploymentRec,
				'transactions': gethProxy.transactionRec,
			}
			with open(os.path.join(BUILD_DIR, 'GethProxyRecords.json'), 'w') as f:
				json.dump(rec, f, indent='\t')


if __name__ == '__main__':
	main()

