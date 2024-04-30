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


THIS_DIR  = os.path.dirname(os.path.abspath(__file__))

ETH_DIR = os.path.abspath(os.path.join(THIS_DIR, 'Ethereum'))
ETH_CLI_CONFIG = os.path.join(ETH_DIR, 'src', 'components_config.json')
ETH_TEST_THR_CONFIG = os.path.join(ETH_DIR, 'tests', 'geth-enclave-throughput-eval', 'components_config.json')

REVOKER_DIR = os.path.abspath(os.path.join(THIS_DIR, 'Revoker'))
REVOKER_CONFIG = os.path.join(REVOKER_DIR, 'src', 'components_config.json')
REVOKER_TEST_E2E_CONFIG = os.path.join(REVOKER_DIR, 'tests', 'End2EndLatency', 'components_config.json')
REVOKER_TEST_PRB_CONFIG = os.path.join(REVOKER_DIR, 'tests', 'ProblematicApp', 'components_config.json')


def LoadConfig(path: str) -> dict:
	with open(path, 'r') as f:
		return json.load(f)


def WriteConfig(path: str, conf: dict) -> None:
	with open(path, 'w') as f:
		json.dump(conf, f, indent='\t')


def UpdDecentServerConf(
	conf: dict,
	hostIp: str,
	port: int,
) -> None:
	for _, cmp in conf['AuthorizedComponents'].items():
		if cmp['Name'] == 'DecentServer':
			cmp['Endpoints']['Inst1']['IP'] = hostIp
			cmp['Endpoints']['Inst1']['Port'] = port


def UpdGethConf(conf: dict, hostIp: str,) -> None:
	conf['Geth']['Host'] = hostIp


def UpdPubSubConf(conf: dict, startBlk: int, addr: str) -> None:
	if 'StartBlock' in conf['PubSub']:
		conf['PubSub']['StartBlock'] = startBlk

	if addr.startswith('0x'):
		addr = addr[2:]

	conf['PubSub']['PubSubAddr'] = addr


def UpdRevokerConf(conf: dict, addr: str) -> None:
	if addr.startswith('0x'):
		addr = addr[2:]
	conf['Publisher']['Addr'] = addr


def main() -> None:
	logging.basicConfig(level=logging.INFO)

	argParse = argparse.ArgumentParser()
	argParse.add_argument(
		'--info', '-i',
		type=str, required=False, default='contract_deploy_info.json',
		help='Contracts deployment info',
	)
	argParse.add_argument(
		'--host-ip',
		type=str, required=True, help='Host IP address',
	)
	argParse.add_argument(
		'--enc-svr-port',
		type=int, required=False, default=10001,
		help='Enclave RA server port',
	)
	argParse.add_argument(
		'--revoker',
		type=str, required=False, default='EnclaveRevokerByConflictMsg',
		help='Revoker name',
	)
	args = argParse.parse_args()

	depInfo = LoadConfig(args.info)
	pubsubInfo = depInfo['PubSub']
	revokerInfo = depInfo[args.revoker]

	ethCltConf = LoadConfig(ETH_CLI_CONFIG)
	UpdDecentServerConf(ethCltConf, args.host_ip, args.enc_svr_port)
	UpdGethConf(ethCltConf, args.host_ip)
	UpdPubSubConf(ethCltConf, pubsubInfo['blockNum'], pubsubInfo['address'])
	WriteConfig(ETH_CLI_CONFIG, ethCltConf)

	ethThrConf = LoadConfig(ETH_TEST_THR_CONFIG)
	UpdDecentServerConf(ethThrConf, args.host_ip, args.enc_svr_port)
	UpdGethConf(ethThrConf, args.host_ip)
	WriteConfig(ETH_TEST_THR_CONFIG, ethThrConf)

	revokerConf = LoadConfig(REVOKER_CONFIG)
	UpdDecentServerConf(revokerConf, args.host_ip, args.enc_svr_port)
	UpdRevokerConf(revokerConf, revokerInfo['address'])
	WriteConfig(REVOKER_CONFIG, revokerConf)

	e2eConf = LoadConfig(REVOKER_TEST_E2E_CONFIG)
	UpdDecentServerConf(e2eConf, args.host_ip, args.enc_svr_port)
	WriteConfig(REVOKER_TEST_E2E_CONFIG, e2eConf)

	probAppConf = LoadConfig(REVOKER_TEST_PRB_CONFIG)
	UpdDecentServerConf(probAppConf, args.host_ip, args.enc_svr_port)
	WriteConfig(REVOKER_TEST_PRB_CONFIG, probAppConf)


if __name__ == '__main__':
	exit(main())


