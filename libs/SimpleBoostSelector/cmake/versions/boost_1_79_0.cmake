# Copyright (c) 2022 SimpleBoostSelector
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.3)

# version info

set(SIMPLEBOOST_DEFAULT_VERSION_TAG
	"boost-1.79.0"
	CACHE STRING
	"Default git tag of the version selected for all sub-libraries"
	FORCE
)
