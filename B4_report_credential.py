#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2024 Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import argparse
import json
import logging
import os
import sys
import time
from typing import Tuple
import web3

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec, utils
from web3 import Web3
from web3.contract.contract import Contract


THIS_DIR  = os.path.dirname(os.path.abspath(__file__))

REVOKER_DIR = os.path.abspath(os.path.join(THIS_DIR, 'Revoker'))
REVOKER_PROB_CRED = os.path.join(REVOKER_DIR, 'build', 'tests', 'ProblematicApp', 'credentials.json')

LIBS_DIR = os.path.abspath(os.path.join(THIS_DIR, 'libs'))

PYHELPER2_DIR = os.path.abspath(os.path.join(LIBS_DIR, 'PyEthHelper2'))

sys.path.append(PYHELPER2_DIR)
from PyEthHelper import EthContractHelper


REVOKE_MSG_HASH = b'REVOKE THIS PRIVATE KEY         '
assert len(REVOKE_MSG_HASH) == 32, 'REVOKE_MSG_HASH must be 32 bytes long'


def GenDecentRevokeSign(credentials: dict) -> Tuple[str, str]:

	privKey: ec.EllipticCurvePrivateKey = serialization.load_der_private_key(
		bytes.fromhex(credentials['privKeyDer']),
		password=None
	)

	sign = privKey.sign(
		REVOKE_MSG_HASH,
		ec.ECDSA(utils.Prehashed(hashes.SHA256()))
	)
	r, s = utils.decode_dss_signature(sign)
	rHex = r.to_bytes(32, 'big').hex()
	sHex = s.to_bytes(32, 'big').hex()

	print('Revoke sign R: {}'.format(rHex))
	print('Revoke sign S: {}'.format(sHex))

	return rHex, sHex


def LoadContract(
	w3: web3.Web3,
	contractInfo: dict,
) -> Contract:

	contract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=(contractInfo['abi'], contractInfo['bin']),
		contractName=contractInfo['name'],
		release=None, # use locally built contract
		address=contractInfo['address'],
	)
	return contract


def RevokeByMessage(
	w3: web3.Web3,
	privKey: str,
	revokerContract: Contract,
	svrCertHex: str,
	credentials: dict,
) -> None:
	if not svrCertHex.startswith('0x'):
		svrCertHex = '0x' + svrCertHex

	EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='reportConflicts',
		arguments=[
			'0x' + credentials['msgIdHash'],
			'0x' + credentials['msgContent1Hash'],
			'0x' + credentials['msg1SignR'],
			'0x' + credentials['msg1SignS'],
			'0x' + credentials['msgContent2Hash'],
			'0x' + credentials['msg2SignR'],
			'0x' + credentials['msg2SignS'],
			svrCertHex,
			'0x' + credentials['appCertDer'],
		 ],
		privKey=privKey,
		confirmPrompt=False # don't prompt for confirmation
	)


def RevokeByLeakedKey(
	w3: web3.Web3,
	privKey: str,
	revokerContract: Contract,
	svrCertHex: str,
	credentials: dict,
) -> None:

	if not svrCertHex.startswith('0x'):
		svrCertHex = '0x' + svrCertHex

	# generate revoke signature
	rHex, sHex = GenDecentRevokeSign(credentials)

	EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='submitRevokeSign',
		arguments=[
			'0x' + rHex,
			'0x' + sHex,
			svrCertHex,
			'0x' + credentials['appCertDer'],
		 ],
		privKey=privKey,
		confirmPrompt=False # don't prompt for confirmation
	)


def GetSeverCertHex(certHexPath: str) -> str:
	with open(certHexPath, 'r') as f:
		svrCertHex = f.read().strip()
	return svrCertHex


def main() -> None:
	logging.basicConfig(level=logging.INFO)

	argParse = argparse.ArgumentParser()
	argParse.add_argument(
		'--geth-addr', '-g',
		type=str, required=True, help='Address to Geth HTTP API',
	)
	argParse.add_argument(
		'--info', '-i',
		type=str, required=False, default='contract_deploy_info.json',
		help='Contracts deployment info',
	)
	argParse.add_argument(
		'--key-file', '-k',
		type=str, required=True, help='Path to file containing testnet keys',
	)
	argParse.add_argument(
		'--revoker',
		type=str, required=False, default='EnclaveRevokerByConflictMsg',
		help='Revoker name',
	)
	argParse.add_argument(
		'--server-cert-path',
		type=str, required=False, default='/etc/decent_svr_cert.hex',
		help='Path to server certificate hex file',
	)
	args = argParse.parse_args()

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

	with open(args.info, 'r') as f:
		contractInfo = json.load(f)

	with open(REVOKER_PROB_CRED, 'r') as f:
		credentials = json.load(f)

	svrCertHex = GetSeverCertHex(args.server_cert_path)

	revokerContract = LoadContract(w3, contractInfo[args.revoker])

	if args.revoker == 'EnclaveRevokerByConflictMsg':
		RevokeByMessage(
			w3=w3,
			privKey=privKey,
			revokerContract=revokerContract,
			svrCertHex=svrCertHex,
			credentials=credentials,
		)
	if args.revoker == 'EnclaveRevokerByLeakedKey':
		RevokeByLeakedKey(
			w3=w3,
			privKey=privKey,
			revokerContract=revokerContract,
			svrCertHex=svrCertHex,
			credentials=credentials,
		)


if __name__ == '__main__':
	exit(main())

