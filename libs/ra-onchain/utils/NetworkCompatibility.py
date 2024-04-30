#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 Decentagram, Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import argparse
import base64
import json
import logging
import os
import sys
import time
from typing import Tuple

from web3 import Web3
from web3.contract.contract import Contract


BASE_DIR_PATH       = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'build')
UTILS_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'utils')
TESTS_DIR           = os.path.join(BASE_DIR_PATH, 'tests')
CERTS_DIR           = os.path.join(TESTS_DIR, 'certs')

BUILD_CONTRACTS_PATH   = os.path.join(BUILD_DIR_PATH, 'contracts')
BUILD_TESTS_PATH       = os.path.join(BUILD_DIR_PATH, 'tests')

LIBS_DIR_PATH       = os.path.join(os.path.dirname(BASE_DIR_PATH))
PYHELPER2_DIR       = os.path.join(LIBS_DIR_PATH, 'PyEthHelper2')

sys.path.insert(0, PYHELPER2_DIR)
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


def _PemToDerCert(certPem: str) -> bytes:
	# PEM to DER
	certPem = certPem.strip()
	certPem = certPem.removeprefix('-----BEGIN CERTIFICATE-----')
	certPem = certPem.removesuffix('-----END CERTIFICATE-----')

	certPem = certPem.replace('\n', '')
	certPem = certPem.replace('\r', '')
	der = base64.b64decode(certPem)

	return der


def LoadIASRootCertDer() -> bytes:
	with open(os.path.join(CERTS_DIR, 'CertIASRoot.pem'), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def LoadIASReportCertDer() -> bytes:
	with open(os.path.join(CERTS_DIR, 'CertIASReport.pem'), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def LoadDecentServerCertDer() -> bytes:
	with open(os.path.join(CERTS_DIR, 'CertDecentServer.pem'), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def LoadDecentAppCertDer() -> bytes:
	with open(os.path.join(CERTS_DIR, 'CertDecentApp.pem'), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


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


	# deploy IASRootCertMgr contract
	contractName = 'IASRootCertMgr'
	contractAbi = os.path.join(BUILD_CONTRACTS_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_CONTRACTS_PATH, f'{contractName}.bin')
	outInfo['IASRootCertMgr'] = {}
	rootCertMgrContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ LoadIASRootCertDer() ],
		outInfo=outInfo['IASRootCertMgr'],
	)


	# deploy IASReportCertMgr contract
	contractName = 'IASReportCertMgr'
	contractAbi = os.path.join(BUILD_CONTRACTS_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_CONTRACTS_PATH, f'{contractName}.bin')
	outInfo['IASReportCertMgr'] = {}
	reportCertMgrContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ rootCertMgrContract.address ],
		outInfo=outInfo['IASReportCertMgr'],
	)

	# verify IAS report certificate
	LOGGER.info('Verifying IAS report certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=reportCertMgrContract,
		funcName='verifyCert',
		arguments=[ LoadIASReportCertDer() ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)


	# deploy DecentServerCertMgr contract
	contractName = 'DecentServerCertMgr'
	contractAbi = os.path.join(BUILD_CONTRACTS_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_CONTRACTS_PATH, f'{contractName}.bin')
	outInfo['DecentServerCertMgr'] = {}
	serverCertMgrContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ reportCertMgrContract.address ],
		outInfo=outInfo['DecentServerCertMgr'],
	)

	# verify Decent server certificate
	LOGGER.info('Verifying Decent Server certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=serverCertMgrContract,
		funcName='verifyCert',
		arguments=[ LoadDecentServerCertDer() ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)


	# deploy HelloWorldApp contract
	contractName = 'HelloWorldApp'
	contractAbi = os.path.join(BUILD_TESTS_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_TESTS_PATH, f'{contractName}.bin')
	outInfo['HelloWorldApp'] = {}
	appContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ serverCertMgrContract.address ],
		outInfo=outInfo['HelloWorldApp'],
	)

	# verify Decent app certificate
	LOGGER.info('Verifying Decent App certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=appContract,
		funcName='loadAppCert',
		arguments=[
			'0xd11169Fe26A678dFb634C67aC85C05ccd796dAEd',
			LoadDecentAppCertDer()
		],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)


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
		type=str, required=False, default='ra_compatibility.log',
		help='Path to the directory where the log file will be stored'
	)
	argParser.add_argument(
		'--info-path', '-i',
		type=str, required=False, default='ra_compatibility_info.json',
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

