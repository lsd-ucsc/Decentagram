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
import binascii
import os
from typing import Union

from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.serialization import Encoding
from cryptography.hazmat.primitives.serialization import PublicFormat
from cryptography.hazmat.primitives.asymmetric.ec import EllipticCurvePublicKey
from cryptography.hazmat.primitives.asymmetric.rsa import RSAPublicKey
from cryptography.hazmat.primitives.asymmetric.types import CertificatePublicKeyTypes


def FormatHex(dataHex: str) -> str:

	# Split into 32 bytes per line
	dataHexLines = [
		dataHex[i:i+64] for i in range(0, len(dataHex), 64)
	]
	dataHex = '"\nhex"'.join(dataHexLines)

	return 'hex"' + dataHex + '"'


def FormatBytes(data: bytes) -> str:
	derHex = data.hex()

	return FormatHex(derHex)


def FormatAndPrintBytes(data: bytes) -> None:
	print(FormatBytes(data))


def FormatInt(data: int) -> str:
	intHex = hex(data)

	# Remove the '0x' prefix
	intHex = intHex[2:]
	# Pad with 0s to make it even length
	if len(intHex) % 2 == 1:
		intHex = '0' + intHex

	return FormatHex(intHex)


def FormatAndPrintInt(data: int) -> None:
	print(FormatInt(data))


def _FormatRSAPubKey(pubKey: RSAPublicKey) -> None:
	num = pubKey.public_numbers()

	print('Public Key Modulus Hex:')
	FormatAndPrintInt(num.n)

	print('Public Key Exponent Hex:')
	FormatAndPrintInt(num.e)


def _FormatEcPubKey(pubKey: EllipticCurvePublicKey) -> None:
	num = pubKey.public_numbers()

	print('Public Key X Hex:')
	FormatAndPrintInt(num.x)

	print('Public Key Y Hex:')
	FormatAndPrintInt(num.y)


def _FormatPubKey(pubKey: CertificatePublicKeyTypes) -> None:
	pubKeyDer = pubKey.public_bytes(
		encoding=Encoding.DER,
		format=PublicFormat.SubjectPublicKeyInfo
	)

	print('Public Key DER Hex:')
	FormatAndPrintBytes(pubKeyDer)

	if isinstance(pubKey, RSAPublicKey):
		_FormatRSAPubKey(pubKey)
	else:
		_FormatEcPubKey(pubKey)


def FormatCert(inputFile: os.PathLike) -> None:
	with open(inputFile, 'r') as f:
		pemStr = f.read()

	# PEM to DER
	cert = x509.load_pem_x509_certificate(pemStr.encode('utf-8'))
	der = cert.public_bytes(encoding=Encoding.DER)

	print('Certificate DER Hex:')
	FormatAndPrintBytes(der)

	print('Certificate TBS Hex:')
	FormatAndPrintBytes(cert.tbs_certificate_bytes)

	digest = hashes.Hash(cert.signature_hash_algorithm)
	digest.update(cert.tbs_certificate_bytes)
	tbsHash = digest.finalize()
	print('Certificate TBS Hash Hex:')
	FormatAndPrintBytes(tbsHash)

	print('Certificate Signature Hex:')
	FormatAndPrintBytes(cert.signature)

	pubKey = cert.public_key()
	_FormatPubKey(pubKey)


def GenerateSecp256k1Key() -> None:
	privKey = ec.generate_private_key(
		curve=ec.SECP256K1(),
	)
	pubKey = privKey.public_key()
	_FormatPubKey(pubKey)


def FormatHexStrOrFile(
	inputFileOrHex: Union[str, os.PathLike],
	startPos: int,
	endPos: int = -1,
) -> None:
	isFilePath = os.path.isfile(inputFileOrHex)
	if isFilePath:
		with open(inputFileOrHex, 'r') as f:
			fileContent = f.read()
		fileContent = fileContent.strip()
		if fileContent.startswith('-----BEGIN CERTIFICATE-----'):
			fileContent = fileContent.removeprefix('-----BEGIN CERTIFICATE-----')
			fileContent = fileContent.removesuffix('-----END CERTIFICATE-----')
		else:
			raise ValueError('Unknown file type')
		fileContent = fileContent.replace('\n', '')
		fileContent = fileContent.replace('\r', '')
		fileBytes = base64.b64decode(fileContent)
		endPos = len(fileBytes) if endPos == -1 else endPos
		resHex = FormatBytes(fileBytes[startPos:endPos])
	else:
		binascii.unhexlify(inputFileOrHex)
		endPos = len(inputFileOrHex) // 2 if endPos == -1 else endPos
		resHex = FormatHex(inputFileOrHex[startPos*2:endPos*2])

	print(resHex)


def main():
	parser = argparse.ArgumentParser(description='Format the test input file.')
	opParser = parser.add_subparsers(
		dest='operation', required=True,
		help='Operation to be performed',
	)
	opParserCert = opParser.add_parser(
		'cert', help='Formatting the certificate'
	)
	opParserCert.add_argument(
		'--input', type=str, required=True,
		help='The input file.'
	)
	opReadHex = opParser.add_parser(
		'hex', help='Formatting the given hex string'
	)
	opReadHex.add_argument(
		'--input', type=str, required=True,
		help='The input file.'
	)
	opReadHex.add_argument(
		'--start', type=int, required=False, default=0,
		help='The start position.'
	)
	opReadHex.add_argument(
		'--end', type=int, required=False, default=-1,
		help='The end position.'
	)
	opReadGen256k1 = opParser.add_parser(
		'gen256k1', help='Generate a secp256k1 key'
	)
	args = parser.parse_args()

	if args.operation == 'cert':
		FormatCert(args.input)
	elif args.operation == 'hex':
		FormatHexStrOrFile(args.input, args.start, args.end)
	elif args.operation == 'gen256k1':
		GenerateSecp256k1Key()
	else:
		raise NotImplementedError(
			f'Operation {args.operation} not implemented'
		)


if __name__ == '__main__':
	main()
