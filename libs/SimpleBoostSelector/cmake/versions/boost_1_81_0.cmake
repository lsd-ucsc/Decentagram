# Copyright (c) 2022 SimpleBoostSelector
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.3)

# version info

set(SIMPLEBOOST_DEFAULT_VERSION_TAG
	"boost-1.81.0"
	CACHE STRING
	"Default git tag of the version selected for all sub-libraries"
	FORCE
)

simpleboost_sublib_depend_override(container_hash
	DEPENDS config
			describe
			mp11
			type_traits
)

simpleboost_sublib_depend_override(fusion
	DEPENDS config container_hash core
			function_types functional
			mpl
			preprocessor
			static_assert
			tuple type_traits typeof
			utility
)
