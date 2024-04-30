#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 ra-onchain
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import hashlib
import json
import os
import requests
import subprocess


LIBS_DIR_PATH = os.path.dirname(os.path.abspath(__file__))
CONFIG_JSON_PATH = os.path.join(LIBS_DIR_PATH, 'sources.json')

REPO_HOST_CONFIGS = {
	'github': {
		'raw_url': 'https://raw.githubusercontent.com/{repo}/{commit}/{path}',
	},
}


def _GenPatchForSingleFile(destFile: str, srcFile: str, verTag: str):
	# print(destFile, '->', srcFile)

	destBaseFile, ext = os.path.splitext(destFile)
	tmpFile = destBaseFile + '.' + verTag + ext
	patchFile = destBaseFile + '.patch'
	destDir = os.path.dirname(destFile)
	# print(tmpFile, '^', destFile, '->', patchFile)

	if os.path.exists(patchFile):
		with open(patchFile, 'r') as f:
			patchFirstLine = f.readline()
			if verTag in patchFirstLine:
				# patch file already exists for this version
				return

	# get dest file content
	with open(destFile, 'rb') as f:
		destContent = f.read()

	# download src file
	resp = requests.get(srcFile, allow_redirects=True)
	if not resp.ok:
		raise RuntimeError('Failed to download src file: {}'.format(srcFile))
	with open(tmpFile, 'wb') as f:
		f.write(resp.content)

	if (
		hashlib.sha256(destContent).digest() !=
		hashlib.sha256(resp.content).digest()
	):
		# generate patch file if content is different
		proc = subprocess.Popen(
			[
				'diff',
				'-u',
				os.path.basename(tmpFile),
				os.path.basename(destFile)
			],
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE,
			cwd=destDir,
		)
		stdout, stderr = proc.communicate()
		# if proc.returncode != 0:
		# 	raise RuntimeError('Failed to generate patch file: {}'.format(stderr))
		with open(patchFile, 'wb') as f:
			f.write(stdout)

	# remove tmp file
	os.remove(tmpFile)


def _GenPatchForFileSrc(filePath: str, src: dict):
	hostConfig = REPO_HOST_CONFIGS[src['host']]
	_GenPatchForSingleFile(
		destFile=os.path.join(LIBS_DIR_PATH, filePath),
		srcFile=hostConfig['raw_url'].format(**src),
		verTag=src['commit'],
	)


def main():
	# read config
	with open(CONFIG_JSON_PATH, 'r') as f:
		configs = json.load(f)

	# generate patch files for file sources
	for item in configs['fileSrc']:
		_GenPatchForFileSrc(item['file'], item['src'])


if __name__ == '__main__':
	main()
