#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 Decentagram, Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###



import argparse
import json
import logging
import os
import random
import sys
import time
from typing import Tuple

from web3 import Web3
from web3.contract.contract import Contract


BASE_DIR_PATH       = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'build')
UTILS_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'utils')

BUILD_PUBSUB_PATH   = os.path.join(BUILD_DIR_PATH, 'PubSub')
BUILD_TESTS_PATH    = os.path.join(BUILD_DIR_PATH, 'tests')

LIBS_DIR_PATH       = os.path.join(os.path.dirname(BASE_DIR_PATH), 'libs')
PYHELPER2_DIR       = os.path.join(LIBS_DIR_PATH, 'PyEthHelper2')

PUBLISH_MSG_GAS_EST = 134745


sys.path.append(PYHELPER2_DIR)
from PyEthHelper import EthContractHelper


LOGGER = logging.getLogger('NetworkCompatibility')


def DeployAndLoadContract(
	w3: Web3,
	privKey: str,
	contractTuple: Tuple[str, str],
	contractName: str,
	arguments: list,
	outInfo: dict,
) -> Contract:

	LOGGER.info(f'Deploying {contractName} contract...')
	contract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=contractTuple,
		contractName=contractName,
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	deployReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=contract,
		arguments=arguments,
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False, # don't prompt for confirmation
	)
	contract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=contractTuple,
		contractName=contractName,
		release=None, # use locally built contract
		address=deployReceipt.contractAddress,
	)
	LOGGER.info(f'{contractName} contract deployed at {deployReceipt.contractAddress}')

	outInfo['address'] = deployReceipt.contractAddress
	outInfo['blockNum'] = deployReceipt.blockNumber
	outInfo['abi'] = contractTuple[0]
	outInfo['bin'] = contractTuple[1]
	outInfo['name'] = contractName
	return contract


def RunTests(apiUrl: str, keyfile: os.PathLike, outInfo: dict) -> dict:
	# connect to endpoint
	w3 = Web3(Web3.HTTPProvider(apiUrl))
	while not w3.is_connected():
		LOGGER.info('Attempting to connect to endpoint...')
		time.sleep(1)
	LOGGER.info('Connected to endpoint')

	# setup account
	privKey = EthContractHelper.SetupSendingAccount(
		w3=w3,
		account=0,
		keyJson=keyfile
	)


	# deploy PubSub contract
	contractName = 'PubSubService'
	contractAbi = os.path.join(BUILD_PUBSUB_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_PUBSUB_PATH, f'{contractName}.bin')
	outInfo['PubSub'] = {}
	pubSubContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ ],
		outInfo=outInfo['PubSub'],
	)

	##########
	# Publisher and register
	##########

	# deploy Publisher contract
	contractName = 'HelloWorldPublisher'
	contractAbi = os.path.join(BUILD_TESTS_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_TESTS_PATH, f'{contractName}.bin')
	outInfo['HelloWorldPublisher'] = {}
	publisherContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ ],
		outInfo=outInfo['HelloWorldPublisher'],
	)

	# register publisher
	LOGGER.info('Registering publisher...')
	regTxReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=publisherContract,
		funcName='register',
		arguments=[ pubSubContract.address ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)

	##########
	# Subscriber and subscribe
	##########

	# deploy Subscriber contract
	contractName = 'HelloWorldSubscriber'
	contractAbi = os.path.join(BUILD_TESTS_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_TESTS_PATH, f'{contractName}.bin')
	outInfo['HelloWorldSubscriber'] = {}
	subscriberContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubContract.address ],
		outInfo=outInfo['HelloWorldSubscriber'],
	)

	# subscribe
	LOGGER.info('Subscribing...')
	gasPrice = int(regTxReceipt.effectiveGasPrice)
	depositEst = gasPrice * PUBLISH_MSG_GAS_EST
	subTxReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=subscriberContract,
		funcName='subscribe',
		arguments=[ publisherContract.address ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=depositEst,
		confirmPrompt=False # prompt for confirmation
	)

	# check if the subscriber was successfully subscribed
	subscribedEvMgrAddr = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=subscriberContract,
		funcName='m_eventMgrAddr',
		arguments=[ ],
		privKey=None,
		gas=None,
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	LOGGER.info(f'Subscriber@{subscriberContract.address} subscribed to EventMgr@{subscribedEvMgrAddr}')
	registeredEvMgrAddr = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=publisherContract,
		funcName='m_eventMgrAddr',
		arguments=[ ],
		privKey=None,
		gas=None,
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	LOGGER.info(f'Publisher@{publisherContract.address} registered with EventMgr@{registeredEvMgrAddr}')
	assert subscribedEvMgrAddr == registeredEvMgrAddr, 'EventMgr addresses do not match'

	LOGGER.info(f'Subscriber@{subscriberContract.address} subscribed to publisher@{publisherContract.address}')

	##########
	# Publish and receive
	##########

	# generate a random message to be published
	expectedMsg = random.randbytes(32).hex()

	# set message to be published
	LOGGER.info('Setting message to be published...')
	EthContractHelper.CallContractFunc(
		w3=w3,
		contract=publisherContract,
		funcName='setSendData',
		arguments=[ expectedMsg ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	LOGGER.info('Message set to "{}"'.format(expectedMsg))

	# estimate the gas limit for publishing
	publishEstGas = (
		100000 + # est gas cost before publishing
		202000 + # gas cost for publishing
		100000   # est gas cost after publishing
	)

	# publish
	LOGGER.info('Publishing...')
	pubTxReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=publisherContract,
		funcName='publish',
		arguments=[ ],
		privKey=privKey,
		gas=publishEstGas,
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)

	# ensure the subscriber received the message
	msg = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=subscriberContract,
		funcName='m_recvData',
		arguments=[ ],
		privKey=None,
		gas=None,
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	LOGGER.info('Message received: "{}"'.format(msg))

	assert msg == expectedMsg, 'Message received does not match the expected message'


def main():
	argParser = argparse.ArgumentParser(
		description='Run tests to check compatibility with a given network'
	)
	argParser.add_argument(
		'--api-url', '-u',
		type=str, required=True,
		help='URL to the JSON-RPC over HTTP API of the network'
	)
	argParser.add_argument(
		'--key-file', '-k',
		type=str, required=True,
		help='Path to the file containing the private keys for the accounts'
	)
	argParser.add_argument(
		'--log-path', '-l',
		type=str, required=False, default='pubsub_compatibility.log',
		help='Path to the directory where the log file will be stored'
	)
	argParser.add_argument(
		'--info-path', '-i',
		type=str, required=False, default='pubsub_compatibility_info.json',
		help='Path to the file containing the deployment information for the contracts'
	)
	args = argParser.parse_args()

	logFormatter = logging.Formatter('[%(asctime)s | %(levelname)s] [%(name)s] %(message)s')
	logLevel = logging.INFO
	rootLogger = logging.root

	rootLogger.setLevel(logLevel)

	consoleHandler = logging.StreamHandler()
	consoleHandler.setFormatter(logFormatter)
	consoleHandler.setLevel(logLevel)
	rootLogger.addHandler(consoleHandler)

	if args.log_path is not None:
		fileHandler = logging.FileHandler(args.log_path)
		fileHandler.setFormatter(logFormatter)
		fileHandler.setLevel(logLevel)
		rootLogger.addHandler(fileHandler)

	info = {}
	RunTests(apiUrl=args.api_url, keyfile=args.key_file, outInfo=info)

	with open(args.info_path, 'w') as f:
		json.dump(info, f, indent='\t')


if __name__ == '__main__':
	exit(main())

