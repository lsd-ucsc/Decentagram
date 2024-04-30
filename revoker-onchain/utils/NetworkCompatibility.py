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
import hashlib
import json
import logging
import os
import random
import sys
import time

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec, utils
from typing import List, Tuple
from web3 import Web3
from web3.contract.contract import Contract


BASE_DIR_PATH       = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'build')
UTILS_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'utils')
TESTS_DIR           = os.path.join(BASE_DIR_PATH, 'tests')
INPUTS_DIR          = os.path.join(TESTS_DIR, 'inputs')

BUILD_ENCR_PATH     = os.path.join(BUILD_DIR_PATH, 'EnclaveRevoker')
BUILD_KEYR_PATH     = os.path.join(BUILD_DIR_PATH, 'KeyRevoker')
BUILD_TESTS_PATH    = os.path.join(BUILD_DIR_PATH, 'tests')

LIBS_DIR_PATH       = os.path.join(os.path.dirname(BASE_DIR_PATH), 'libs')
PYHELPER2_DIR       = os.path.join(LIBS_DIR_PATH, 'PyEthHelper2')

sys.path.append(PYHELPER2_DIR)
from PyEthHelper import EthContractHelper



REVOKE_MSG_HASH = b'REVOKE THIS PRIVATE KEY         '
assert len(REVOKE_MSG_HASH) == 32, 'REVOKE_MSG_HASH must be 32 bytes long'


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


##########
# Enclave credential helpers
##########


def GenEnclaveRevokeSign(credentials: dict) -> Tuple[str, str]:

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

	LOGGER.info('Revoke sign R: {}'.format(rHex))
	LOGGER.info('Revoke sign S: {}'.format(sHex))

	return rHex, sHex


def LoadIASRootCertDer() -> bytes:
	with open(os.path.join(INPUTS_DIR, 'CertIASRoot.pem'), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def LoadDecentSvrCertDer(idx: int) -> bytes:
	filename = 'CertDecentServer_{:02}.pem'.format(idx)
	with open(os.path.join(INPUTS_DIR, filename), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def LoadDecentAppCertDer(sIdx: int, aIdx: int) -> bytes:
	filename = 'CertDecentApp_S{:02}_{:02}.pem'.format(sIdx, aIdx)
	with open(os.path.join(INPUTS_DIR, filename), 'r') as f:
		certPem = f.read()

	return _PemToDerCert(certPem)


def LoadProblemCredential(sIdx: int, aIdx: int, mIdx: int) -> dict:
	filename = 'CredProbApp_S{:02}_{:02}_{:02}.json'.format(sIdx, aIdx, mIdx)
	with open(os.path.join(INPUTS_DIR, filename), 'r') as f:
		msg = json.load(f)

	return msg


def GenerateRevokeSign(credentials: dict) -> Tuple[str, str]:

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

	LOGGER.info('Revoke sign R: {}'.format(rHex))
	LOGGER.info('Revoke sign S: {}'.format(sHex))

	return rHex, sHex


##########
# Key credential helpers
##########


def GenConflictMsg() -> dict:
	privKey = ec.generate_private_key(ec.SECP256K1())
	pubKey = privKey.public_key()

	pubKeyX = pubKey.public_numbers().x
	pubKeyY = pubKey.public_numbers().y
	pubKeyBytes = pubKeyX.to_bytes(32, 'big') + pubKeyY.to_bytes(32, 'big')
	pubKeyAddr = Web3.to_checksum_address(
		Web3.keccak(pubKeyBytes)[-20:].hex()
	)

	res = {
		'eventId': random.randbytes(16),
		'msg1': random.randbytes(32),
		'msg2': random.randbytes(32),
		'pubKeyBytes': pubKeyBytes.hex(),
		'pubKeyAddr': pubKeyAddr
	}

	res['eventIdHash'] = hashlib.sha256(res['eventId']).digest()
	res['msg1Hash'] = hashlib.sha256(res['msg1']).digest()
	res['msg2Hash'] = hashlib.sha256(res['msg2']).digest()

	msg1Sign = privKey.sign(
		res['eventIdHash'] + res['msg1Hash'],
		ec.ECDSA(hashes.SHA256())
	)
	r, s = utils.decode_dss_signature(msg1Sign)
	res['msg1SignR'] = '0x' + r.to_bytes(32, 'big').hex()
	res['msg1SignS'] = '0x' + s.to_bytes(32, 'big').hex()

	msg2Sign = privKey.sign(
		res['eventIdHash'] + res['msg2Hash'],
		ec.ECDSA(hashes.SHA256())
	)
	r, s = utils.decode_dss_signature(msg2Sign)
	res['msg2SignR'] = '0x' + r.to_bytes(32, 'big').hex()
	res['msg2SignS'] = '0x' + s.to_bytes(32, 'big').hex()

	res['eventIdHash'] = '0x' + res['eventIdHash'].hex()
	res['msg1Hash'] = '0x' + res['msg1Hash'].hex()
	res['msg2Hash'] = '0x' + res['msg2Hash'].hex()

	LOGGER.info('eventIdHash: ' + res['eventIdHash'])
	LOGGER.info('msg1Hash:    ' + res['msg1Hash'])
	LOGGER.info('msg1SignR:   ' + res['msg1SignR'])
	LOGGER.info('msg1SignS:   ' + res['msg1SignS'])
	LOGGER.info('msg2Hash:    ' + res['msg2Hash'])
	LOGGER.info('msg2SignR:   ' + res['msg2SignR'])
	LOGGER.info('msg2SignS:   ' + res['msg2SignS'])
	LOGGER.info('signerAddr:  ' + res['pubKeyAddr'])

	return res


def GenKeyRevokeSign() -> dict:
	privKey = ec.generate_private_key(ec.SECP256K1())
	pubKey = privKey.public_key()

	pubKeyX = pubKey.public_numbers().x
	pubKeyY = pubKey.public_numbers().y
	pubKeyBytes = pubKeyX.to_bytes(32, 'big') + pubKeyY.to_bytes(32, 'big')
	pubKeyAddr = Web3.to_checksum_address(
		Web3.keccak(pubKeyBytes)[-20:].hex()
	)

	res = {
		'pubKeyBytes': pubKeyBytes.hex(),
		'pubKeyAddr': pubKeyAddr
	}

	sign = privKey.sign(
		REVOKE_MSG_HASH,
		ec.ECDSA(utils.Prehashed(hashes.SHA256()))
	)
	r, s = utils.decode_dss_signature(sign)

	res['revokeSignR'] = '0x' + r.to_bytes(32, 'big').hex()
	res['revokeSignS'] = '0x' + s.to_bytes(32, 'big').hex()

	LOGGER.info('Revoke sign R: {}'.format(res['revokeSignR']))
	LOGGER.info('Revoke sign S: {}'.format(res['revokeSignS']))
	LOGGER.info('Signer addr:   {}'.format(res['pubKeyAddr']))

	return res


##########
# Tests - Voting
##########


def DeployVoteProxyContract(
	w3: Web3,
	privKey: str,
	numVoters: int = 3,
	outInfo: dict = None,
) -> List[Contract]:
	proxyContracts = []
	outInfo['VoteProxy'] = []
	for i in range(numVoters):
		# deploy VoteProxy contract
		contractName = 'VoteProxy'
		contractAbi = os.path.join(BUILD_TESTS_PATH, f'{contractName}.abi')
		contractBin = os.path.join(BUILD_TESTS_PATH, f'{contractName}.bin')
		info = {}
		proxyContract = DeployAndLoadContract(
			w3=w3,
			privKey=privKey,
			contractTuple=(contractAbi, contractBin),
			contractName=contractName,
			arguments=[ ],
			outInfo=info,
		)
		outInfo['VoteProxy'].append(info)
		proxyContracts.append(proxyContract)

	return proxyContracts


def RunRevokerByVotingTests(
	w3: Web3,
	privKey: str,
	pubSubAddr: str,
	proxyContracts: List[Contract],
	outInfo: dict = None,
) -> None:

	stakeholders = [ proxyContract.address for proxyContract in proxyContracts ]
	LOGGER.info(f'Stakeholders are: {stakeholders}')

	# deploy RevokerByVoting contract
	contractName = 'RevokerByVoting'
	contractAbi = os.path.join(BUILD_ENCR_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_ENCR_PATH, f'{contractName}.bin')
	outInfo['RevokerByVoting'] = {}
	votingContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubAddr, stakeholders ],
		outInfo=outInfo['RevokerByVoting'],
	)


	# revoke an enclave
	enclaveId = '0x1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF'


	for proxyContract in proxyContracts:
		LOGGER.info(f'{proxyContract.address} casting vote')
		voteReceipt = EthContractHelper.CallContractFunc(
			w3=w3,
			contract=proxyContract,
			funcName='enclaveRevokeVote',
			arguments=[ votingContract.address, enclaveId ],
			privKey=privKey,
			confirmPrompt=False # don't prompt for confirmation
		)

	revokeState = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=votingContract,
		funcName='isRevoked',
		arguments=[ enclaveId ],
		privKey=None,
		confirmPrompt=False # don't prompt for confirmation
	)
	assert revokeState == True, 'Enclave should be revoked by now'


def RunKeyRevokerByVotingTests(
	w3: Web3,
	privKey: str,
	pubSubAddr: str,
	proxyContracts: List[Contract],
	outInfo: dict = None,
) -> None:

	stakeholders = [ proxyContract.address for proxyContract in proxyContracts ]
	LOGGER.info(f'Stakeholders are: {stakeholders}')

	# deploy KeyRevokerByVoting contract
	contractName = 'KeyRevokerByVoting'
	contractAbi = os.path.join(BUILD_KEYR_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_KEYR_PATH, f'{contractName}.bin')
	outInfo['KeyRevokerByVoting'] = {}
	votingContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubAddr, stakeholders ],
		outInfo=outInfo['KeyRevokerByVoting'],
	)


	# revoke some random key address
	keyAddr = GenKeyRevokeSign()['pubKeyAddr']


	for proxyContract in proxyContracts:
		LOGGER.info(f'{proxyContract.address} casting vote')
		voteReceipt = EthContractHelper.CallContractFunc(
			w3=w3,
			contract=proxyContract,
			funcName='keyRevokeVote',
			arguments=[ votingContract.address, keyAddr ],
			privKey=privKey,
			confirmPrompt=False # don't prompt for confirmation
		)

	revokeState = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=votingContract,
		funcName='isRevoked',
		arguments=[ keyAddr ],
		privKey=None,
		confirmPrompt=False # don't prompt for confirmation
	)
	assert revokeState == True, 'Key should be revoked by now'


##########
# Tests - Conflicting Messages
##########


def RunRevokerByConflictMsgTests(
	w3: Web3,
	privKey: str,
	pubSubAddr: str,
	decentSvrCertMgrAddr: str,
	outInfo: dict = None,
) -> None:
	# deploy RevokerByConflictMsg contract
	contractName = 'RevokerByConflictMsg'
	contractAbi = os.path.join(BUILD_ENCR_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_ENCR_PATH, f'{contractName}.bin')
	outInfo['RevokerByConflictMsg'] = {}
	revokerContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubAddr, decentSvrCertMgrAddr ],
		outInfo=outInfo['RevokerByConflictMsg'],
	)

	credentials = LoadProblemCredential(0, 0, 1)
	LOGGER.info('report conflict msg to revoke enclave {}'.format(
		credentials['enclaveHash']
	))
	reportReceipt = EthContractHelper.CallContractFunc(
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
			LoadDecentSvrCertDer(0),
			LoadDecentAppCertDer(0, 0),
		 ],
		privKey=privKey,
		confirmPrompt=False # don't prompt for confirmation
	)

	revokeState = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='isRevoked',
		arguments=[ '0x' + credentials['enclaveHash'] ],
		privKey=None,
		confirmPrompt=False # don't prompt for confirmation
	)
	assert revokeState == True, 'Enclave should be revoked'


def RunKeyRevokerByConflictMsgTests(
	w3: Web3,
	privKey: str,
	pubSubAddr: str,
	outInfo: dict = None,
) -> None:

	# deploy KeyRevokerByConflictMsg contract
	contractName = 'KeyRevokerByConflictMsg'
	contractAbi = os.path.join(BUILD_KEYR_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_KEYR_PATH, f'{contractName}.bin')
	outInfo['KeyRevokerByConflictMsg'] = {}
	revokerContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubAddr, ],
		outInfo=outInfo['KeyRevokerByConflictMsg'],
	)

	credentials = GenConflictMsg()
	LOGGER.info('report conflict msg to revoke key address {}'.format(
		credentials['pubKeyAddr']
	))
	reportReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='reportConflicts',
		arguments=[
			credentials['eventIdHash'],
			credentials['msg1Hash'],
			credentials['msg1SignR'],
			credentials['msg1SignS'],
			credentials['msg2Hash'],
			credentials['msg2SignR'],
			credentials['msg2SignS'],
			credentials['pubKeyAddr'],
		 ],
		privKey=privKey,
		confirmPrompt=False # don't prompt for confirmation
	)

	revokeState = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='isRevoked',
		arguments=[ credentials['pubKeyAddr'] ],
		privKey=None,
		confirmPrompt=False # don't prompt for confirmation
	)
	assert revokeState == True, 'key address should be revoked'


##########
# Tests - Leaked Keys
##########


def RunRevokerByLeakedKeyTests(
	w3: Web3,
	privKey: str,
	pubSubAddr: str,
	decentSvrCertMgrAddr: str,
	outInfo: dict = None,
) -> None:
	# deploy RevokerByLeakedKey contract
	contractName = 'RevokerByLeakedKey'
	contractAbi = os.path.join(BUILD_ENCR_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_ENCR_PATH, f'{contractName}.bin')
	outInfo['RevokerByLeakedKey'] = {}
	revokerContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubAddr, decentSvrCertMgrAddr, ],
		outInfo=outInfo['RevokerByLeakedKey'],
	)


	credentials = LoadProblemCredential(0, 0, 1)


	# generate revoke signature
	rHex, sHex = GenEnclaveRevokeSign(credentials)

	LOGGER.info('report leaked key to revoke enclave {}'.format(
		credentials['enclaveHash']
	))
	reportReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='submitRevokeSign',
		arguments=[
			'0x' + rHex,
			'0x' + sHex,
			LoadDecentSvrCertDer(0),
			LoadDecentAppCertDer(0, 0),
		 ],
		privKey=privKey,
		confirmPrompt=False # don't prompt for confirmation
	)

	revokeState = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='isRevoked',
		arguments=[ '0x' + credentials['enclaveHash'] ],
		privKey=None,
		confirmPrompt=False # don't prompt for confirmation
	)
	assert revokeState == True, 'Enclave should be revoked'


def RunKeyRevokerByLeakedKeyTests(
	w3: Web3,
	privKey: str,
	pubSubAddr: str,
	outInfo: dict = None,
) -> None:
	# deploy KeyRevokerByLeakedKey contract
	contractName = 'KeyRevokerByLeakedKey'
	contractAbi = os.path.join(BUILD_KEYR_PATH, f'{contractName}.abi')
	contractBin = os.path.join(BUILD_KEYR_PATH, f'{contractName}.bin')
	outInfo['KeyRevokerByLeakedKey'] = {}
	revokerContract = DeployAndLoadContract(
		w3=w3,
		privKey=privKey,
		contractTuple=(contractAbi, contractBin),
		contractName=contractName,
		arguments=[ pubSubAddr, ],
		outInfo=outInfo['KeyRevokerByLeakedKey'],
	)

	credentials = GenKeyRevokeSign()

	LOGGER.info('report leaked key to revoke key address {}'.format(
		credentials['pubKeyAddr']
	))
	reportReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='submitRevokeSign',
		arguments=[
			credentials['revokeSignR'],
			credentials['revokeSignS'],
			credentials['pubKeyAddr'],
		 ],
		privKey=privKey,
		confirmPrompt=False # don't prompt for confirmation
	)

	revokeState = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=revokerContract,
		funcName='isRevoked',
		arguments=[ credentials['pubKeyAddr'] ],
		privKey=None,
		confirmPrompt=False # don't prompt for confirmation
	)
	assert revokeState == True, 'key address should be revoked'


def RunTests(
	apiUrl: str,
	keyfile: os.PathLike,
	pubSubAddr: str,
	decentSvrCertMgrAddr: str,
	outInfo: dict = None,
) -> dict:
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

	LOGGER.info(f'Using PubSubService at {pubSubAddr}')
	LOGGER.info(f'Using DecentSvrCertMgr at {decentSvrCertMgrAddr}')

	pxyContracts = DeployVoteProxyContract(
		w3=w3,
		privKey=privKey,
		numVoters=3,
		outInfo=outInfo,
	)
	RunRevokerByVotingTests(
		w3=w3,
		privKey=privKey,
		pubSubAddr=pubSubAddr,
		proxyContracts=pxyContracts,
		outInfo=outInfo,
	)
	RunKeyRevokerByVotingTests(
		w3=w3,
		privKey=privKey,
		pubSubAddr=pubSubAddr,
		proxyContracts=pxyContracts,
		outInfo=outInfo,
	)

	RunRevokerByConflictMsgTests(
		w3=w3,
		privKey=privKey,
		pubSubAddr=pubSubAddr,
		decentSvrCertMgrAddr=decentSvrCertMgrAddr,
		outInfo=outInfo,
	)
	RunKeyRevokerByConflictMsgTests(
		w3=w3,
		privKey=privKey,
		pubSubAddr=pubSubAddr,
		outInfo=outInfo,
	)

	RunRevokerByLeakedKeyTests(
		w3=w3,
		privKey=privKey,
		pubSubAddr=pubSubAddr,
		decentSvrCertMgrAddr=decentSvrCertMgrAddr,
		outInfo=outInfo,
	)
	RunKeyRevokerByLeakedKeyTests(
		w3=w3,
		privKey=privKey,
		pubSubAddr=pubSubAddr,
		outInfo=outInfo,
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
		type=str, required=False, default='revoker_compatibility.log',
		help='Path to the directory where the log file will be stored'
	)
	argParser.add_argument(
		'--info-path', '-i',
		type=str, required=False, default='revoker_compatibility_info.json',
		help='Path to the file containing the deployment information for the contracts'
	)
	argParser.add_argument(
		'--pubsub-info', '-p',
		type=str, required=False, default='pubsub_compatibility_info.json',
		help='Path to the file containing the deployment information for the PubSubService contract'
	)
	argParser.add_argument(
		'--ra-info', '-r',
		type=str, required=False, default='ra_compatibility_info.json',
		help='Path to the file containing the deployment information for the DecentSvrCertMgr contract'
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

	with open(args.pubsub_info, 'r') as f:
		pubSubInfo = json.load(f)
		pubSubAddr = pubSubInfo['PubSub']['address']

	with open(args.ra_info, 'r') as f:
		raInfo = json.load(f)
		decentSvrCertMgrAddr = raInfo['DecentServerCertMgr']['address']

	info = {}
	RunTests(
		apiUrl=args.api_url,
		keyfile=args.key_file,
		pubSubAddr=pubSubAddr,
		decentSvrCertMgrAddr=decentSvrCertMgrAddr,
		outInfo=info,
	)

	with open(args.info_path, 'w') as f:
		json.dump(info, f, indent='\t')


if __name__ == '__main__':
	exit(main())

