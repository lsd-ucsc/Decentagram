#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 ra-onchain
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import argparse
import base64
import json
import logging
import os
import signal
import subprocess
import sys
import time

from web3 import Web3


ROOT_DIR     = os.path.join(os.path.dirname(__file__), '..')
UTILS_DIR    = os.path.join(ROOT_DIR, 'utils')
BUILD_DIR    = os.path.join(ROOT_DIR, 'build')
TESTS_DIR    = os.path.join(ROOT_DIR, 'tests')
CERTS_DIR    = os.path.join(TESTS_DIR, 'certs')
PYHELPER_DIR = os.path.join(UTILS_DIR, 'PyEthHelper')
PROJECT_CONFIG_PATH = os.path.join(UTILS_DIR, 'project_conf.json')
CHECKSUM_KEYS_PATH  = os.path.join(BUILD_DIR, 'ganache_keys_checksum.json')
GANACHE_KEYS_PATH   = os.path.join(BUILD_DIR, 'ganache_keys.json')
GANACHE_PORT     = 7545
GANACHE_NUM_KEYS = 20
GANACHE_NET_ID   = 1337


sys.path.append(PYHELPER_DIR)
from PyEthHelper import EthContractHelper
from PyEthHelper import GanacheAccounts


def StartGanache() -> subprocess.Popen:
	cmd = [
		'ganache-cli',
		'-p', str(GANACHE_PORT),
		'-d',
		'-a', str(GANACHE_NUM_KEYS),
		'--network-id', str(GANACHE_NET_ID),
		'--chain.hardfork', 'shanghai',
		'--wallet.accountKeysPath', str(GANACHE_KEYS_PATH),
	]
	proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

	return proc


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


def RunTests_VerifyIndividually() -> dict:
	# connect to ganache
	ganacheUrl = 'http://localhost:{}'.format(GANACHE_PORT)
	w3 = Web3(Web3.HTTPProvider(ganacheUrl))
	while not w3.is_connected():
		print('Attempting to connect to ganache...')
		time.sleep(1)
	print('Connected to ganache')

	# checksum keys
	GanacheAccounts.ChecksumGanacheKeysFile(
		CHECKSUM_KEYS_PATH,
		GANACHE_KEYS_PATH
	)

	# setup account
	privKey = EthContractHelper.SetupSendingAccount(
		w3=w3,
		account=0, # use account 0
		keyJson=CHECKSUM_KEYS_PATH
	)

	gasCosts = {}

	# deploy IASRootCertMgr contract
	print('Deploying IASRootCertMgr contract...')
	iasRootContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='IASRootCertMgr',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	iasRootReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=iasRootContract,
		arguments=[ LoadIASRootCertDer() ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	iasRootAddr = iasRootReceipt.contractAddress
	print('IASRootCertMgr contract deployed at {}'.format(iasRootAddr))
	print()

	gasCosts['deployIasRootCertMgr'] = iasRootReceipt.gasUsed

	# deploy IASReportCertMgr contract
	print('Deploying IASReportCertMgr contract...')
	iasReportContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='IASReportCertMgr',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	iasReportReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=iasReportContract,
		arguments=[ iasRootAddr ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	iasReportAddr = iasReportReceipt.contractAddress
	print('IASReportCertMgr contract deployed at {}'.format(iasReportAddr))
	iasReportContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='IASReportCertMgr',
		release=None, # use locally built contract
		address=iasReportAddr
	)
	print()

	gasCosts['deployIasReportCertMgr'] = iasReportReceipt.gasUsed

	# verify IAS report certificate
	print('Verifying IAS report certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=iasReportContract,
		funcName='verifyCert',
		arguments=[ LoadIASReportCertDer() ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	print()

	gasCosts['verifyIasReportCert'] = verifyReceipt.gasUsed

	# deploy DecentServerCertMgr contract
	print('Deploying DecentServerCertMgr contract...')
	decentSvrContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='DecentServerCertMgr',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	decentSvrReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=decentSvrContract,
		arguments=[ iasReportAddr ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	decentSvrAddr = decentSvrReceipt.contractAddress
	print('DecentServerCertMgr contract deployed at {}'.format(decentSvrAddr))
	decentSvrContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='DecentServerCertMgr',
		release=None, # use locally built contract
		address=decentSvrAddr
	)
	print()

	gasCosts['deployDecentServerCertMgr'] = decentSvrReceipt.gasUsed

	# verify Decent server certificate
	print('Verifying Decent Server certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=decentSvrContract,
		funcName='verifyCert',
		arguments=[ LoadDecentServerCertDer() ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	print()

	gasCosts['verifyDecentServerCert'] = verifyReceipt.gasUsed

	# deploy HelloWorldApp contract
	print('Deploying HelloWorldApp contract...')
	decentAppContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='HelloWorldApp',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	decentAppReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=decentAppContract,
		arguments=[ decentSvrAddr ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	decentAppAddr = decentAppReceipt.contractAddress
	print('HelloWorldApp contract deployed at {}'.format(decentAppAddr))
	decentAppContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='HelloWorldApp',
		release=None, # use locally built contract
		address=decentAppAddr
	)
	print()

	gasCosts['deployHelloWorldApp'] = decentAppReceipt.gasUsed

	# verify Decent server certificate
	print('Verifying Decent App certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=decentAppContract,
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
	print()

	gasCosts['verifyDecentAppCert'] = verifyReceipt.gasUsed

	return gasCosts


def RunTests_VerifyAllOnce() -> None:
	# connect to ganache
	ganacheUrl = 'http://localhost:{}'.format(GANACHE_PORT)
	w3 = Web3(Web3.HTTPProvider(ganacheUrl))
	while not w3.is_connected():
		print('Attempting to connect to ganache...')
		time.sleep(1)
	print('Connected to ganache')

	# checksum keys
	GanacheAccounts.ChecksumGanacheKeysFile(
		CHECKSUM_KEYS_PATH,
		GANACHE_KEYS_PATH
	)

	# setup account
	privKey = EthContractHelper.SetupSendingAccount(
		w3=w3,
		account=0, # use account 0
		keyJson=CHECKSUM_KEYS_PATH
	)

	gasCosts = {}

	# deploy IASRootCertMgr contract
	print('Deploying IASRootCertMgr contract...')
	iasRootContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='IASRootCertMgr',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	iasRootReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=iasRootContract,
		arguments=[ LoadIASRootCertDer() ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	iasRootAddr = iasRootReceipt.contractAddress
	print('IASRootCertMgr contract deployed at {}'.format(iasRootAddr))
	print()

	# deploy IASReportCertMgr contract
	print('Deploying IASReportCertMgr contract...')
	iasReportContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='IASReportCertMgr',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	iasReportReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=iasReportContract,
		arguments=[ iasRootAddr ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	iasReportAddr = iasReportReceipt.contractAddress
	print('IASReportCertMgr contract deployed at {}'.format(iasReportAddr))
	print()

	# deploy DecentServerCertMgr contract
	print('Deploying DecentServerCertMgr contract...')
	decentSvrContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='DecentServerCertMgr',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	decentSvrReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=decentSvrContract,
		arguments=[ iasReportAddr ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	decentSvrAddr = decentSvrReceipt.contractAddress
	print('DecentServerCertMgr contract deployed at {}'.format(decentSvrAddr))
	print()

	# deploy HelloWorldApp contract
	print('Deploying HelloWorldApp contract...')
	decentAppContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='HelloWorldApp',
		release=None, # use locally built contract
		address=None, # deploy new contract
	)
	decentAppReceipt = EthContractHelper.DeployContract(
		w3=w3,
		contract=decentAppContract,
		arguments=[ decentSvrAddr ],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	decentAppAddr = decentAppReceipt.contractAddress
	print('HelloWorldApp contract deployed at {}'.format(decentAppAddr))
	decentAppContract = EthContractHelper.LoadContract(
		w3=w3,
		projConf=PROJECT_CONFIG_PATH,
		contractName='HelloWorldApp',
		release=None, # use locally built contract
		address=decentAppAddr
	)
	print()

	# verify Decent certificate chain
	print('Verifying Decent App certificate...')
	verifyReceipt = EthContractHelper.CallContractFunc(
		w3=w3,
		contract=decentAppContract,
		funcName='verifyCertChain',
		arguments=[
			LoadDecentServerCertDer(),
			LoadDecentAppCertDer()
		],
		privKey=privKey,
		gas=None, # let web3 estimate
		value=0,
		confirmPrompt=False # don't prompt for confirmation
	)
	print()

	gasCosts['verifyCertChain'] = verifyReceipt.gasUsed

	return gasCosts


def EvaluateGasCosts() -> None:
	gasCosts1 = RunTests_VerifyIndividually()
	gasCosts2 = RunTests_VerifyAllOnce()
	gasCost = {
		**gasCosts1,
		**gasCosts2,
	}
	savePath = os.path.join(BUILD_DIR, 'gas_costs.json')
	with open(savePath, 'w') as f:
		json.dump(gasCost, f, indent='\t')


def StopGanache(ganacheProc: subprocess.Popen) -> None:
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
	print('Ganache has been shut down')


def main():
	argParser = argparse.ArgumentParser(
		description='Deploy contracts and run tests'
	)
	modParser = argParser.add_subparsers(
		dest='testMode',
		help='Test mode',
		required=True
	)
	modParser.add_parser(
		'indiv',
		help='Verify certificates individually'
	)
	modParser.add_parser(
		'all',
		help='Verify certificates all at once'
	)
	modParser.add_parser(
		'eval',
		help='Evaluate gas costs and save results to a JSON file'
	)
	args = argParser.parse_args()

	# logging configuration
	loggingFormat = '%(asctime)s %(levelname)s %(message)s'
	logging.basicConfig(level=logging.INFO, format=loggingFormat)
	# logger = logging.getLogger(__name__ + main.__name__)

	ganacheProc = StartGanache()

	try:
		if args.testMode == 'indiv':
			RunTests_VerifyIndividually()
		elif args.testMode == 'all':
			RunTests_VerifyAllOnce()
		elif args.testMode == 'eval':
			EvaluateGasCosts()
		else:
			raise RuntimeError('Unexpected test mode: {}'.format(args.testMode))
	finally:
		# finish and exit
		StopGanache(ganacheProc)


if __name__ == '__main__':
	main()
