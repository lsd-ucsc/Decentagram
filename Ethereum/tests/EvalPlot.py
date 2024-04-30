#!/usr/bin/env python3
# -*- coding:utf-8 -*-
###
# Copyright (c) 2023 Decentagram
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.
###


import json
import os
import time

from typing import List, Tuple, Union

import plotly.graph_objects as go
import plotly.express as px

from plotly.subplots import make_subplots


BASE_DIR_PATH       = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR_PATH      = os.path.join(BASE_DIR_PATH, 'build')


ErrorBarData = List[int]
SingleData   = int
DataPoint    = Tuple[int, Union[SingleData, ErrorBarData]]
DataPoints   = List[DataPoint]


# available markers: https://plotly.com/python/marker-style/
# circle, square, diamond, cross, x, triangle, pentagon, hexagram, star, diamond, hourglass, bowtie, asterisk, hash, y
# available colors: https://plotly.com/python/discrete-color/
POINT_MARKERS = [
	{
		'size': 8,
		'symbol': 'circle',
		'color': px.colors.qualitative.Plotly[0],
	},
	{
		'size': 8,
		'symbol': 'square',
		'color': px.colors.qualitative.Plotly[1],
	},
	{
		'size': 8,
		'symbol': 'diamond',
		'color': px.colors.qualitative.Plotly[2],
	},
	{
		'size': 8,
		'symbol': 'cross',
		'color': px.colors.qualitative.Plotly[3],
	},
]


def GenerateFigure(
	inData: List[DataPoints],
	dataNames: List[str],
	isSecondary: List[bool],
	xLabel: str,
	yLabel: str,
	title: str,
	marks: List[dict] = POINT_MARKERS,
	y2ndLabel: Union[str, None] = None,
) -> go.Figure:

	fig = make_subplots(specs=[[{"secondary_y": True}]])

	# setup margin
	topSpace = 50 if title else 10
	fig.update_layout(
		autosize=True,
		margin={ 'l': 10, 'r': 10, 't': topSpace, 'b': 10, }
	)

	# setup legend, title, and axis labels
	fig.update_layout(
		legend=dict(
			yanchor="top",
			y=0.99,
			xanchor="left",
			x=0.25
		),
		title=title,
		xaxis_title=xLabel,
	)
	fig.update_yaxes(
		title_text=yLabel,
		secondary_y=False,
	)
	if y2ndLabel:
		fig.update_yaxes(
			title_text=y2ndLabel,
			secondary_y=True,
		)

	# plot data
	for ds, dName, is2nd, mark in zip(inData, dataNames, isSecondary, marks):
		hasErrBar = type(ds[0][1]) == list
		if hasErrBar:
			yNormal = [ y[1] for _, y in ds ]
			yPlus   = [ y[2] - y[1] for _, y in ds ]
			yMinus  = [ y[1] - y[0] for _, y in ds ]
			fig.add_trace(
				go.Scatter(
					x=[ x    for x, _ in ds ],
					y=yNormal,
					error_y=dict(
						type='data',
						array=yPlus,
						arrayminus=yMinus,
					),
					name=dName,
					mode='lines+markers',
					marker=mark,
				),
				secondary_y=is2nd,
			)
		else:
			fig.add_trace(
				go.Scatter(
					x=[ x for x, _ in ds ],
					y=[ y for _, y in ds ],
					name=dName,
					mode='lines+markers',
					marker=mark,
				),
				secondary_y=is2nd,
			)

	#fig.update_xaxes(tickmode='linear')
	fig.update_yaxes(tickformat="none")

	return fig


def SaveFigure(
	fig: go.Figure,
	outName: str,
) -> None:
	fig.write_image(outName + '.svg')
	# fig.write_image(outName + '.png')

	fig.write_image(outName + '.pdf')
	# mitigation for issue https://github.com/plotly/plotly.py/issues/3469
	time.sleep(2)
	fig.write_image(outName + '.pdf')


def PlotGraph(
	inData: List[DataPoints],
	dataNames: List[str],
	isSecondary: List[bool],
	xLabel: str,
	yLabel: str,
	title: str,
	outName: str,
	marks: List[dict] = POINT_MARKERS,
	y2ndLabel: Union[str, None] = None,
) -> None:
	fig = GenerateFigure(
		inData,
		dataNames,
		isSecondary,
		xLabel,
		yLabel,
		title,
		marks,
		y2ndLabel,
	)

	SaveFigure(fig, outName)


def ReadResults(
	inputPath: os.PathLike,
) -> DataPoints:
	with open(inputPath, 'r') as f:
		results = json.load(f)

	# convert list of 3 test results to results with min & max
	assert len(results) == 3, 'Expected 3 test results'
	assert len(results[0]) == len(results[1]) == len(results[2]), \
		'Expected 3 test results with same length'

	processedResults = []
	for testRes1, testRes2, testRes3 in zip(*results):
		assert testRes1[0] == testRes2[0] == testRes3[0], \
			'Expected 3 test results with same X axis value'

		processedResults.append((
			testRes1[0], # x axis value
			sorted(
				[
					testRes1[1],
					testRes2[1],
					testRes3[1],
				]
			) # y axis value, in ascending order
		))

	# check if it is necessary to keep min & max
	for _, ys in processedResults:
		if not(ys[0] == ys[1] == ys[2]):
			# need to keep min & max
			return processedResults

	# all 3 results are the same
	# no need to keep min & max
	return [
		(x, y[1])
		for x, y in processedResults
	]


def ToThroughputResults(
	inData: DataPoints,
	numBlocks: int,
) -> DataPoints:
	throughputRes = []

	if isinstance(inData[0][1], list):
		# has error bar
		for pair in inData:
			ys = pair[1]
			throughputRes.append([
				pair[0],
				sorted([
					numBlocks / ys[0],
					numBlocks / ys[1],
					numBlocks / ys[2],
				])
			])
	else:
		# no error bar
		for pair in inData:
			throughputRes.append([
				pair[0],
				numBlocks / pair[1]
			])

	return throughputRes


def main() -> None:
	NUM_OF_BLOCKS = 5000

	#===== Go Ethereum =====#
	goTimeEval = ReadResults(
		os.path.join(BUILD_DIR_PATH, 'go_eval.json')
	)
	goThroughputEval = ToThroughputResults(
		goTimeEval,
		NUM_OF_BLOCKS,
	)

	PlotGraph(
		inData=[ goTimeEval, goThroughputEval, ],
		dataNames=[ 'Go Ethereum Time Elapsed', 'Go Ethereum Throughput', ],
		isSecondary=[ False, True, ],
		title='Go Ethereum Block Processing Evaluation'
				f' (over {NUM_OF_BLOCKS} blocks)',
		xLabel='Percentage of Transaction Receipts being Verified (%)',
		yLabel='Time Elapsed (Seconds)',
		outName=os.path.join(BUILD_DIR_PATH, 'go_eval'),
		y2ndLabel='Throughput (Blocks/Second)',
	)

	#===== Decentagram Ethereum =====#
	decentTimeEval = ReadResults(
		os.path.join(BUILD_DIR_PATH, 'decent_eval.json')
	)
	decentThroughputEval = ToThroughputResults(
		decentTimeEval,
		NUM_OF_BLOCKS,
	)

	PlotGraph(
		inData=[ decentTimeEval, decentThroughputEval, ],
		dataNames=[
			'Enclave Impl. Time Elapsed',
			'Enclave Impl. Throughput',
		],
		isSecondary=[ False, True, ],
		title='Decentagram Ethereum Block Processing Evaluation'
				f' (over {NUM_OF_BLOCKS} blocks)',
		xLabel='Percentage of Transaction Receipts being Verified (%)',
		yLabel='Time Elapsed (Seconds)',
		outName=os.path.join(BUILD_DIR_PATH, 'decent_eval'),
		y2ndLabel='Throughput (Blocks/Second)',
	)


	#===== Summary Graph =====#
	PlotGraph(
		inData=[
			goTimeEval,
			goThroughputEval,
			decentTimeEval,
			decentThroughputEval,
		],
		dataNames=[
			'Go Ethereum Time Elapsed',
			'Go Ethereum Throughput',
			'Enclave Impl. Time Elapsed',
			'Enclave Impl. Throughput',
		],
		isSecondary=[
			False,
			True,
			False,
			True,
		],
		title='Receipts Processing Evaluation'
				f' (over {NUM_OF_BLOCKS} blocks)',
		xLabel='Percentage of Transaction Receipts being Verified (%)',
		yLabel='Time Elapsed (Seconds)',
		outName=os.path.join(BUILD_DIR_PATH, 'all_eval'),
		y2ndLabel='Throughput (Blocks/Second)',
	)


if __name__ == '__main__':
	main()
