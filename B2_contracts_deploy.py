#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2024 Decentagram
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
import web3

from web3 import Web3
from web3.contract.contract import Contract
from typing import Tuple


THIS_DIR  = os.path.dirname(os.path.abspath(__file__))

ETH_DIR = os.path.abspath(os.path.join(THIS_DIR, 'Ethereum'))

REVOKER_DIR = os.path.abspath(os.path.join(THIS_DIR, 'Revoker'))

PUBSUB_OC_DIR = os.path.abspath(os.path.join(THIS_DIR, 'pubsub-onchain'))
PUBSUB_OC_BUILD_PUBSUB_DIR = os.path.join(PUBSUB_OC_DIR, 'build', 'PubSub')

REVOKER_OC_DIR = os.path.abspath(os.path.join(THIS_DIR, 'revoker-onchain'))
REVOKER_OC_BUILD_ENC_DIR = os.path.join(REVOKER_OC_DIR, 'build', 'EnclaveRevoker')
REVOKER_OC_BUILD_KEY_DIR = os.path.join(REVOKER_OC_DIR, 'build', 'KeyRevoker')

LIBS_DIR = os.path.abspath(os.path.join(THIS_DIR, 'libs'))

RA_OC_DIR = os.path.abspath(os.path.join(LIBS_DIR, 'ra-onchain'))
RA_OC_BUILD_CON_DIR = os.path.join(RA_OC_DIR, 'build', 'contracts')
RA_OC_BUILD_CERT_DIR = os.path.join(RA_OC_DIR, 'tests', 'certs')

PYHELPER2_DIR = os.path.abspath(os.path.join(LIBS_DIR, 'PyEthHelper2'))

sys.path.append(PYHELPER2_DIR)
from PyEthHelper import EthContractHelper


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
	with open(os.path.join(RA_OC_BUILD_CERT_DIR, 'CertIASRoot.pem'), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def DeployAndLoadContract(
	w3: web3.Web3,
	privKey: str,
	contractTuple: Tuple[str, str],
	contractName: str,
	arguments: list,
	outInfo: dict,
) -> Contract:

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
	outInfo['address'] = deployReceipt.contractAddress
	outInfo['blockNum'] = deployReceipt.blockNumber
	outInfo['abi'] = contractTuple[0]
	outInfo['bin'] = contractTuple[1]
	outInfo['name'] = contractName
	return contract


def DeployPubSubContract(
	w3: web3.Web3,
	privKey: str,
	outInfo: dict,
) -> None:

	outInfo['PubSub'] = {}
	DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(
			os.path.join(PUBSUB_OC_BUILD_PUBSUB_DIR, 'PubSubService' + '.abi'),
			os.path.join(PUBSUB_OC_BUILD_PUBSUB_DIR, 'PubSubService' + '.bin'),
		),
		contractName='PubSubService',
		arguments=[ ],
		outInfo=outInfo['PubSub'],
	)


def DeployIASRootCertMgr(
	w3: web3.Web3,
	privKey: str,
	outInfo: dict,
) -> None:

	outInfo['IASRootCertMgr'] = {}
	DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(
			os.path.join(RA_OC_BUILD_CON_DIR, 'IASRootCertMgr' + '.abi'),
			os.path.join(RA_OC_BUILD_CON_DIR, 'IASRootCertMgr' + '.bin'),
		),
		contractName='IASRootCertMgr',
		arguments=[ LoadIASRootCertDer() ],
		outInfo=outInfo['IASRootCertMgr'],
	)


def DeployIASReportCertMgr(
	w3: web3.Web3,
	privKey: str,
	outInfo: dict,
	iasRootAddr: str,
) -> None:

	outInfo['IASReportCertMgr'] = {}
	DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(
			os.path.join(RA_OC_BUILD_CON_DIR, 'IASReportCertMgr' + '.abi'),
			os.path.join(RA_OC_BUILD_CON_DIR, 'IASReportCertMgr' + '.bin'),
		),
		contractName='IASReportCertMgr',
		arguments=[ iasRootAddr ],
		outInfo=outInfo['IASReportCertMgr'],
	)


def DeployDecentServerCertMgr(
	w3: web3.Web3,
	privKey: str,
	outInfo: dict,
	iasReportAddr: str,
) -> None:

	outInfo['DecentServerCertMgr'] = {}
	DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(
			os.path.join(RA_OC_BUILD_CON_DIR, 'DecentServerCertMgr' + '.abi'),
			os.path.join(RA_OC_BUILD_CON_DIR, 'DecentServerCertMgr' + '.bin'),
		),
		contractName='DecentServerCertMgr',
		arguments=[ iasReportAddr ],
		outInfo=outInfo['DecentServerCertMgr'],
	)


def DeployEnclaveRevokerMsgContract(
	w3: web3.Web3,
	privKey: str,
	outInfo: dict,
	pubsubAddr: str,
	decentSvrCertMgrAddr: str,
) -> None:

	outInfo['EnclaveRevokerByConflictMsg'] = {}
	DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(
			os.path.join(REVOKER_OC_BUILD_ENC_DIR, 'RevokerByConflictMsg' + '.abi'),
			os.path.join(REVOKER_OC_BUILD_ENC_DIR, 'RevokerByConflictMsg' + '.bin'),
		),
		contractName='RevokerByConflictMsg',
		arguments=[ pubsubAddr, decentSvrCertMgrAddr ],
		outInfo=outInfo['EnclaveRevokerByConflictMsg'],
	)


def DeployEnclaveRevokerKeyContract(
	w3: web3.Web3,
	privKey: str,
	outInfo: dict,
	pubsubAddr: str,
	decentSvrCertMgrAddr: str,
) -> None:

	outInfo['EnclaveRevokerByLeakedKey'] = {}
	DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(
			os.path.join(REVOKER_OC_BUILD_ENC_DIR, 'RevokerByLeakedKey' + '.abi'),
			os.path.join(REVOKER_OC_BUILD_ENC_DIR, 'RevokerByLeakedKey' + '.bin'),
		),
		contractName='RevokerByLeakedKey',
		arguments=[ pubsubAddr, decentSvrCertMgrAddr ],
		outInfo=outInfo['EnclaveRevokerByLeakedKey'],
	)


def main() -> None:
	logging.basicConfig(level=logging.INFO)

	argParse = argparse.ArgumentParser()
	argParse.add_argument(
		'--geth-addr', '-g',
		type=str, required=True, help='Address to Geth HTTP API',
	)
	argParse.add_argument(
		'--key-file', '-k',
		type=str, required=True, help='Path to file containing testnet keys',
	)
	args = argParse.parse_args()

	outInfo = {}

	w3 = Web3(Web3.HTTPProvider(args.geth_addr))
	while not w3.is_connected():
		print('Attempting to connect to Geth Client...')
		time.sleep(1)
	print('Connected to Geth Client')

	privKey = EthContractHelper.SetupSendingAccount(
		w3=w3,
		account=0, # use account 0
		keyJson=args.key_file,
	)

	DeployPubSubContract(w3=w3, privKey=privKey, outInfo=outInfo)

	DeployIASRootCertMgr(w3=w3, privKey=privKey, outInfo=outInfo)
	DeployIASReportCertMgr(w3=w3, privKey=privKey, outInfo=outInfo,
						iasRootAddr=outInfo['IASRootCertMgr']['address'])
	DeployDecentServerCertMgr(w3=w3, privKey=privKey, outInfo=outInfo,
						iasReportAddr=outInfo['IASReportCertMgr']['address'])

	DeployEnclaveRevokerMsgContract(w3=w3, privKey=privKey, outInfo=outInfo,
						pubsubAddr=outInfo['PubSub']['address'],
						decentSvrCertMgrAddr=outInfo['DecentServerCertMgr']['address'])
	DeployEnclaveRevokerKeyContract(w3=w3, privKey=privKey, outInfo=outInfo,
						pubsubAddr=outInfo['PubSub']['address'],
						decentSvrCertMgrAddr=outInfo['DecentServerCertMgr']['address'])

	with open(os.path.join(THIS_DIR, 'contract_deploy_info.json'), 'w') as outF:
		json.dump(outInfo, outF, indent='\t')


if __name__ == '__main__':
	exit(main())
