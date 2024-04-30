# Copyright (c) 2022 SimpleBoostSelector
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.14)

include_guard()

#     ###
#    ## ##
#   ##   ##
#  ##     ##
#  #########
#  ##     ##
#  ##     ##

simpleboost_add_sublib(accumulators
	TARGET_NAME     Boost::accumulators
	GIT_ADDR_HTTPS  "https://github.com/boostorg/accumulators.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/accumulators.git"
	DEPENDS array assert
			circular_buffer concept_check config core
			fusion
			iterator
			mpl
			numeric_conversion numeric_ublas
			parameter preprocessor
			range
			serialization static_assert
			throw_exception tuple type_traits typeof
)

simpleboost_add_sublib(algorithm
	TARGET_NAME     Boost::algorithm
	GIT_ADDR_HTTPS  "https://github.com/boostorg/algorithm.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/algorithm.git"
	DEPENDS array assert
			bind
			concept_check config core
			exception
			function
			iterator
			mpl
			range regex
			static_assert
			throw_exception tuple type_traits
			unordered
)

simpleboost_add_sublib(align
	TARGET_NAME     Boost::align
	GIT_ADDR_HTTPS  "https://github.com/boostorg/align.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/align.git"
	DEPENDS assert
			config core
			static_assert
)

simpleboost_add_sublib(any
	TARGET_NAME     Boost::any
	GIT_ADDR_HTTPS  "https://github.com/boostorg/any.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/any.git"
	DEPENDS config core
			static_assert
			throw_exception type_index type_traits
)

simpleboost_add_sublib(array
	TARGET_NAME     Boost::array
	GIT_ADDR_HTTPS  "https://github.com/boostorg/array.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/array.git"
	DEPENDS assert
			config core
			static_assert
			throw_exception
)

simpleboost_add_sublib(asio
	TARGET_NAME     Boost::asio
	GIT_ADDR_HTTPS  "https://github.com/boostorg/asio.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/asio.git"
	DEPENDS align array assert
			bind
			chrono config core coroutine
			date_time
			function
			regex
			smart_ptr system
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(assert
	TARGET_NAME     Boost::assert
	GIT_ADDR_HTTPS  "https://github.com/boostorg/assert.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/assert.git"
	DEPENDS config
)

simpleboost_add_sublib(assign
	TARGET_NAME     Boost::assign
	GIT_ADDR_HTTPS  "https://github.com/boostorg/assign.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/assign.git"
	DEPENDS array
			config core
			move mpl
			preprocessor ptr_container
			range
			static_assert
			throw_exception tuple type_traits
)

simpleboost_add_sublib(atomic
	TARGET_NAME     Boost::atomic
	GIT_ADDR_HTTPS  "https://github.com/boostorg/atomic.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/atomic.git"
	DEPENDS align assert
			config
			predef preprocessor
			static_assert
			type_traits
	DEPENDS_WIN32 winapi
)

#  ########
#  ##     ##
#  ##     ##
#  ########
#  ##     ##
#  ##     ##
#  ########

simpleboost_add_sublib(beast
	TARGET_NAME     Boost::beast
	GIT_ADDR_HTTPS  "https://github.com/boostorg/beast.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/beast.git"
	DEPENDS asio assert
			bind
			config container core
			endian
			intrusive
			logic
			mp11
			optional
			smart_ptr static_assert system
			throw_exception type_traits
			utility
			winapi
)

simpleboost_add_sublib(bimap
	TARGET_NAME     Boost::bimap
	GIT_ADDR_HTTPS  "https://github.com/boostorg/bimap.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/bimap.git"
	DEPENDS concept_check config container_hash core
			iterator
			lambda
			mpl multi_index
			preprocessor
			serialization static_assert
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(bind
	TARGET_NAME     Boost::bind
	GIT_ADDR_HTTPS  "https://github.com/boostorg/bind.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/bind.git"
	DEPENDS config core
)

#   ######
#  ##    ##
#  ##
#  ##
#  ##
#  ##    ##
#   ######

simpleboost_add_sublib(callable_traits
	TARGET_NAME     Boost::callable_traits
	GIT_ADDR_HTTPS  "https://github.com/boostorg/callable_traits.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/callable_traits.git"
)

simpleboost_add_sublib(chrono
	TARGET_NAME     Boost::chrono
	GIT_ADDR_HTTPS  "https://github.com/boostorg/chrono.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/chrono.git"
	DEPENDS assert
			config core
			integer
			move mpl
			predef
			ratio
			static_assert system
			throw_exception type_traits typeof
			utility
			winapi
)

simpleboost_add_sublib(circular_buffer
	TARGET_NAME     Boost::circular_buffer
	GIT_ADDR_HTTPS  "https://github.com/boostorg/circular_buffer.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/circular_buffer.git"
	DEPENDS assert
			concept_check config core
			move
			static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(compatibility
	TARGET_NAME     Boost::compatibility
	GIT_ADDR_HTTPS  "https://github.com/boostorg/compatibility.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/compatibility.git"
)

simpleboost_add_sublib(compute
	TARGET_NAME     Boost::compute
	GIT_ADDR_HTTPS  "https://github.com/boostorg/compute.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/compute.git"
	DEPENDS algorithm array assert atomic
			chrono config core
			filesystem function function_types fusion
			iterator
			lexical_cast
			mpl
			optional
			preprocessor property_tree proto
			range
			smart_ptr static_assert
			thread throw_exception tuple type_traits typeof
			utility uuid
)

simpleboost_add_sublib(concept_check
	TARGET_NAME     Boost::concept_check
	GIT_ADDR_HTTPS  "https://github.com/boostorg/concept_check.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/concept_check.git"
	DEPENDS config preprocessor
			static_assert type_traits
)

simpleboost_add_sublib(config
	TARGET_NAME     Boost::config
	GIT_ADDR_HTTPS  "https://github.com/boostorg/config.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/config.git"
)

simpleboost_add_sublib(container
	TARGET_NAME     Boost::container
	GIT_ADDR_HTTPS  "https://github.com/boostorg/container.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/container.git"
	DEPENDS assert
			config core
			intrusive
			move
			static_assert
			type_traits
			winapi
)

simpleboost_add_sublib(container_hash
	TARGET_NAME     Boost::container_hash
	GIT_ADDR_HTTPS  "https://github.com/boostorg/container_hash.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/container_hash.git"
	DEPENDS assert
			config core
			detail
			integer
			static_assert
			type_traits
)

simpleboost_add_sublib(context
	TARGET_NAME     Boost::context
	GIT_ADDR_HTTPS  "https://github.com/boostorg/context.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/context.git"
	DEPENDS assert
			config core
			mp11
			pool predef
			smart_ptr
)

simpleboost_add_sublib(contract
	TARGET_NAME     Boost::contract
	GIT_ADDR_HTTPS  "https://github.com/boostorg/contract.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/contract.git"
	DEPENDS any assert
			config core
			exception
			function function_types
			mpl
			optional
			preprocessor
			smart_ptr static_assert
			thread type_traits typeof
			utility
)

simpleboost_add_sublib(conversion
	TARGET_NAME     Boost::conversion
	GIT_ADDR_HTTPS  "https://github.com/boostorg/conversion.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/conversion.git"
	DEPENDS assert
			config core
			smart_ptr
			throw_exception type_traits typeof
)

simpleboost_add_sublib(convert
	TARGET_NAME     Boost::convert
	GIT_ADDR_HTTPS  "https://github.com/boostorg/convert.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/convert.git"
	DEPENDS config core
			function_types
			lexical_cast
			math mpl
			optional
			parameter
			range
			spirit
			type_traits
)

simpleboost_add_sublib(core
	TARGET_NAME     Boost::core
	GIT_ADDR_HTTPS  "https://github.com/boostorg/core.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/core.git"
	DEPENDS assert
			config
			static_assert
			throw_exception
)

simpleboost_add_sublib(coroutine
	TARGET_NAME     Boost::coroutine
	GIT_ADDR_HTTPS  "https://github.com/boostorg/coroutine.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/coroutine.git"
	DEPENDS assert
			config context core
			exception
			move
			system
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(coroutine2
	TARGET_NAME     Boost::coroutine2
	GIT_ADDR_HTTPS  "https://github.com/boostorg/coroutine2.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/coroutine2.git"
	DEPENDS assert
			config context
)

simpleboost_add_sublib(crc
	TARGET_NAME     Boost::crc
	GIT_ADDR_HTTPS  "https://github.com/boostorg/crc.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/crc.git"
	DEPENDS array
			config
			integer
			type_traits
)

#  ########
#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#  ########

simpleboost_add_sublib(date_time
	TARGET_NAME     Boost::date_time
	GIT_ADDR_HTTPS  "https://github.com/boostorg/date_time.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/date_time.git"
	DEPENDS algorithm assert
			config core
			io
			lexical_cast
			numeric_conversion
			range
			smart_ptr static_assert
			throw_exception tokenizer type_traits
			utility
			winapi
)

simpleboost_add_sublib(describe
	TARGET_NAME     Boost::describe
	GIT_ADDR_HTTPS  "https://github.com/boostorg/describe.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/describe.git"
	DEPENDS mp11
)

simpleboost_add_sublib(detail
	TARGET_NAME     Boost::detail
	GIT_ADDR_HTTPS  "https://github.com/boostorg/detail.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/detail.git"
	DEPENDS config core
			preprocessor
			static_assert
			type_traits
)

simpleboost_add_sublib(dll
	TARGET_NAME     Boost::dll
	GIT_ADDR_HTTPS  "https://github.com/boostorg/dll.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/dll.git"
	DEPENDS assert
			config core
			filesystem function
			move
			predef
			smart_ptr spirit static_assert system
			throw_exception type_index type_traits
			winapi
)

simpleboost_add_sublib(dynamic_bitset
	TARGET_NAME     Boost::dynamic_bitset
	GIT_ADDR_HTTPS  "https://github.com/boostorg/dynamic_bitset.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/dynamic_bitset.git"
	DEPENDS assert
			config container_hash core
			integer
			move
			static_assert
			throw_exception
)

#  ########
#  ##
#  ##
#  ######
#  ##
#  ##
#  ########

simpleboost_add_sublib(endian
	TARGET_NAME     Boost::endian
	GIT_ADDR_HTTPS  "https://github.com/boostorg/endian.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/endian.git"
	DEPENDS config core
			static_assert
			type_traits
)

simpleboost_add_sublib(exception
	TARGET_NAME     Boost::exception
	GIT_ADDR_HTTPS  "https://github.com/boostorg/exception.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/exception.git"
	DEPENDS assert
			config core
			smart_ptr
			throw_exception tuple type_traits
)

#  ########
#  ##
#  ##
#  ######
#  ##
#  ##
#  ##

simpleboost_add_sublib(fiber
	TARGET_NAME     Boost::fiber
	GIT_ADDR_HTTPS  "https://github.com/boostorg/fiber.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/fiber.git"
	DEPENDS assert
			config context core
			intrusive
			predef
			smart_ptr
)

simpleboost_add_sublib(fiber_numa
	TARGET_NAME     Boost::fiber_numa
	GIT_ADDR_HTTPS  "https://github.com/boostorg/fiber.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/fiber.git"
	DEPENDS assert algorithm
			config context
			fiber filesystem format
			smart_ptr
)

simpleboost_add_sublib(filesystem
	TARGET_NAME     Boost::filesystem
	GIT_ADDR_HTTPS  "https://github.com/boostorg/filesystem.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/filesystem.git"
	DEPENDS assert
			config container_hash core
			detail
			io iterator
			predef
			smart_ptr system
			type_traits
)

simpleboost_add_sublib(flyweight
	TARGET_NAME     Boost::flyweight
	GIT_ADDR_HTTPS  "https://github.com/boostorg/flyweight.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/flyweight.git"
	DEPENDS assert
			config container_hash core
			detail
			interprocess
			mpl multi_index
			parameter preprocessor
			serialization smart_ptr
			throw_exception type_traits
)

simpleboost_add_sublib(foreach
	TARGET_NAME     Boost::foreach
	GIT_ADDR_HTTPS  "https://github.com/boostorg/foreach.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/foreach.git"
	DEPENDS config core
			iterator
			mpl
			range
			type_traits
)

simpleboost_add_sublib(format
	TARGET_NAME     Boost::format
	GIT_ADDR_HTTPS  "https://github.com/boostorg/format.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/format.git"
	DEPENDS assert
			config core
			optional
			smart_ptr
			throw_exception
			utility
)

simpleboost_add_sublib(function
	TARGET_NAME     Boost::function
	GIT_ADDR_HTTPS  "https://github.com/boostorg/function.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/function.git"
	DEPENDS assert
			bind
			config core
			integer
			preprocessor
			throw_exception type_index type_traits typeof
)

simpleboost_add_sublib(function_types
	TARGET_NAME     Boost::function_types
	GIT_ADDR_HTTPS  "https://github.com/boostorg/function_types.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/function_types.git"
	DEPENDS config core
			detail
			mpl
			preprocessor
			type_traits
)

simpleboost_add_sublib(functional
	TARGET_NAME     Boost::functional
	GIT_ADDR_HTTPS  "https://github.com/boostorg/functional.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/functional.git"
	DEPENDS config core
			function function_types
			mpl
			preprocessor
			type_traits typeof
			utility
)

simpleboost_add_sublib(fusion
	TARGET_NAME     Boost::fusion
	GIT_ADDR_HTTPS  "https://github.com/boostorg/fusion.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/fusion.git"
	DEPENDS config container_hash core
			function_types
			mpl
			preprocessor
			static_assert
			tuple type_traits typeof
			utility
)

#  ####
#   ##
#   ##
#   ##
#   ##
#   ##
#  ####

simpleboost_add_sublib(icl
	TARGET_NAME     Boost::icl
	GIT_ADDR_HTTPS  "https://github.com/boostorg/icl.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/icl.git"
	DEPENDS assert
			concept_check config container core
			date_time detail
			iterator
			move mpl
			range rational
			static_assert
			type_traits
			utility
)

simpleboost_add_sublib(integer
	TARGET_NAME     Boost::integer
	GIT_ADDR_HTTPS  "https://github.com/boostorg/integer.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/integer.git"
	DEPENDS assert
			config core
			static_assert
			throw_exception
)

simpleboost_add_sublib(interprocess
	TARGET_NAME     Boost::interprocess
	GIT_ADDR_HTTPS  "https://github.com/boostorg/interprocess.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/interprocess.git"
	DEPENDS assert
			config container core
			integer intrusive
			move
			static_assert
			type_traits
			unordered
			winapi
)

simpleboost_add_sublib(intrusive
	TARGET_NAME     Boost::intrusive
	GIT_ADDR_HTTPS  "https://github.com/boostorg/intrusive.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/intrusive.git"
	DEPENDS assert
			config container_hash core
			move
			static_assert
)

simpleboost_add_sublib(io
	TARGET_NAME     Boost::io
	GIT_ADDR_HTTPS  "https://github.com/boostorg/io.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/io.git"
	DEPENDS config
)

simpleboost_add_sublib(iostreams
	TARGET_NAME     Boost::iostreams
	GIT_ADDR_HTTPS  "https://github.com/boostorg/iostreams.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/iostreams.git"
	DEPENDS assert
			config core
			detail
			function
			integer iterator
			mpl
			numeric_conversion
			preprocessor
			random range regex
			smart_ptr static_assert
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(iterator
	TARGET_NAME     Boost::iterator
	GIT_ADDR_HTTPS  "https://github.com/boostorg/iterator.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/iterator.git"
	DEPENDS assert
			concept_check config conversion core
			detail
			function_types fusion
			mpl
			optional
			smart_ptr static_assert
			type_traits
			utility
)

#        ##
#        ##
#        ##
#        ##
#  ##    ##
#  ##    ##
#   ######

simpleboost_add_sublib(json
	TARGET_NAME     Boost::json
	GIT_ADDR_HTTPS  "https://github.com/boostorg/json.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/json.git"
	DEPENDS align assert
			config container
			exception
			mp11
			system
			throw_exception
			utility
)

#  ##
#  ##
#  ##
#  ##
#  ##
#  ##
#  ########

simpleboost_add_sublib(lambda
	TARGET_NAME     Boost::lambda
	GIT_ADDR_HTTPS  "https://github.com/boostorg/lambda.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/lambda.git"
	DEPENDS bind
			config core
			detail
			iterator
			mpl
			preprocessor
			tuple type_traits
			utility
)

simpleboost_add_sublib(lambda2
	TARGET_NAME     Boost::lambda2
	GIT_ADDR_HTTPS  "https://github.com/boostorg/lambda2.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/lambda2.git"
)

simpleboost_add_sublib(leaf
	TARGET_NAME     Boost::leaf
	GIT_ADDR_HTTPS  "https://github.com/boostorg/leaf.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/leaf.git"
)

simpleboost_add_sublib(lexical_cast
	TARGET_NAME     Boost::lexical_cast
	GIT_ADDR_HTTPS  "https://github.com/boostorg/lexical_cast.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/lexical_cast.git"
	DEPENDS array assert
			config container core
			integer
			numeric_conversion
			range
			static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(local_function
	TARGET_NAME     Boost::local_function
	GIT_ADDR_HTTPS  "https://github.com/boostorg/local_function.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/local_function.git"
	DEPENDS config
			mpl
			preprocessor
			scope_exit
			type_traits typeof
			utility
)

simpleboost_add_sublib(locale
	TARGET_NAME     Boost::locale
	GIT_ADDR_HTTPS  "https://github.com/boostorg/locale.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/locale.git"
	DEPENDS assert
			config
			function
			iterator
			smart_ptr static_assert
			thread type_traits
			unordered
)

simpleboost_add_sublib(lockfree
	TARGET_NAME     Boost::lockfree
	GIT_ADDR_HTTPS  "https://github.com/boostorg/lockfree.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/lockfree.git"
	DEPENDS align array assert atomic
			config core
			integer iterator
			mpl
			parameter predef
			static_assert
			tuple type_traits
			utility
)

simpleboost_add_sublib(log
	TARGET_NAME     Boost::log
	GIT_ADDR_HTTPS  "https://github.com/boostorg/log.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/log.git"
	DEPENDS align array asio assert
			bind
			config container core
			date_time
			exception
			filesystem function_types fusion
			interprocess intrusive
			lexical_cast
			move mpl
			optional
			parameter phoenix predef preprocessor proto
			random range
			smart_ptr spirit static_assert system
			throw_exception type_index type_traits
			utility
			winapi
)

simpleboost_add_sublib(logic
	TARGET_NAME     Boost::logic
	GIT_ADDR_HTTPS  "https://github.com/boostorg/logic.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/logic.git"
	DEPENDS config core
)

#  ##     ##
#  ###   ###
#  #### ####
#  ## ### ##
#  ##     ##
#  ##     ##
#  ##     ##

simpleboost_add_sublib(math
	TARGET_NAME     Boost::math
	GIT_ADDR_HTTPS  "https://github.com/boostorg/math.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/math.git"
	DEPENDS assert
			concept_check config core
			integer
			lexical_cast
			predef
			random
			static_assert
			throw_exception
)

simpleboost_add_sublib(metaparse
	TARGET_NAME     Boost::metaparse
	GIT_ADDR_HTTPS  "https://github.com/boostorg/metaparse.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/metaparse.git"
	DEPENDS config
			mpl
			predef preprocessor
			static_assert
			type_traits
)

simpleboost_add_sublib(move
	TARGET_NAME     Boost::move
	GIT_ADDR_HTTPS  "https://github.com/boostorg/move.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/move.git"
	DEPENDS assert
			config core
			static_assert
)

simpleboost_add_sublib(mp11
	TARGET_NAME     Boost::mp11
	GIT_ADDR_HTTPS  "https://github.com/boostorg/mp11.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/mp11.git"
)

simpleboost_add_sublib(mpi
	TARGET_NAME     Boost::mpi
	GIT_ADDR_HTTPS  "https://github.com/boostorg/mpi.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/mpi.git"
	DEPENDS assert config core
			foreach function
			graph
			integer iterator
			lexical_cast
			mpl
			optional
			serialization smart_ptr static_assert
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(mpl
	TARGET_NAME     Boost::mpl
	GIT_ADDR_HTTPS  "https://github.com/boostorg/mpl.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/mpl.git"
	DEPENDS config core
			predef preprocessor
			static_assert
			type_traits
			utility
)

simpleboost_add_sublib(msm
	TARGET_NAME     Boost::msm
	GIT_ADDR_HTTPS  "https://github.com/boostorg/msm.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/msm.git"
	DEPENDS any assert
			bind
			circular_buffer config core
			function fusion
			mpl
			parameter phoenix preprocessor proto
			serialization
			tuple type_traits typeof
)

simpleboost_add_sublib(multi_array
	TARGET_NAME     Boost::multi_array
	GIT_ADDR_HTTPS  "https://github.com/boostorg/multi_array.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/multi_array.git"
	DEPENDS array assert
			concept_check config core
			functional
			iterator
			mpl
			static_assert
			type_traits
)

simpleboost_add_sublib(multi_index
	TARGET_NAME     Boost::multi_index
	GIT_ADDR_HTTPS  "https://github.com/boostorg/multi_index.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/multi_index.git"
	DEPENDS assert
			bind
			config container_hash core
			foreach
			integer iterator
			move mpl
			preprocessor
			serialization smart_ptr static_assert
			throw_exception tuple type_traits
			utility
)

simpleboost_add_sublib(multiprecision
	TARGET_NAME     Boost::multiprecision
	GIT_ADDR_HTTPS  "https://github.com/boostorg/multiprecision.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/multiprecision.git"
	DEPENDS assert
			config core
			integer
			lexical_cast
			math
			random
)

#  ##    ## ##     ## ##     ## ######## ########  ####  ######
#  ###   ## ##     ## ###   ### ##       ##     ##  ##  ##    ##
#  ####  ## ##     ## #### #### ##       ##     ##  ##  ##
#  ## ## ## ##     ## ## ### ## ######   ########   ##  ##
#  ##  #### ##     ## ##     ## ##       ##   ##    ##  ##
#  ##   ### ##     ## ##     ## ##       ##    ##   ##  ##    ##
#  ##    ##  #######  ##     ## ######## ##     ## ####  ######

simpleboost_add_sublib(numeric_conversion
	TARGET_NAME     Boost::numeric_conversion
	GIT_ADDR_HTTPS  "https://github.com/boostorg/numeric_conversion.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/numeric_conversion.git"
	DEPENDS config conversion core
			mpl
			preprocessor
			throw_exception type_traits
)

simpleboost_add_sublib(numeric_interval
	TARGET_NAME     Boost::numeric_interval
	GIT_ADDR_HTTPS  "https://github.com/boostorg/interval.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/interval.git"
	DEPENDS config
			detail
			logic
)

#   #######
#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#   #######

simpleboost_add_sublib(optional
	TARGET_NAME     Boost::optional
	GIT_ADDR_HTTPS  "https://github.com/boostorg/optional.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/optional.git"
	DEPENDS assert
			config core
			detail
			move
			predef
			static_assert
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(outcome
	TARGET_NAME     Boost::outcome
	GIT_ADDR_HTTPS  "https://github.com/boostorg/outcome.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/outcome.git"
	DEPENDS config
			exception
			system
			throw_exception
)

#  ########
#  ##     ##
#  ##     ##
#  ########
#  ##
#  ##
#  ##

simpleboost_add_sublib(parameter
	TARGET_NAME     Boost::parameter
	GIT_ADDR_HTTPS  "https://github.com/boostorg/parameter.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/parameter.git"
	DEPENDS config core
			function fusion
			mp11 mpl
			optional
			preprocessor
			type_traits
			utility
)

simpleboost_add_sublib(parameter_python
	TARGET_NAME     Boost::parameter_python
	GIT_ADDR_HTTPS  "https://github.com/boostorg/parameter_python.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/parameter_python.git"
	DEPENDS mpl
			parameter preprocessor python
)

simpleboost_add_sublib(pfr
	TARGET_NAME     Boost::pfr
	GIT_ADDR_HTTPS  "https://github.com/boostorg/pfr.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/pfr.git"
)

simpleboost_add_sublib(phoenix
	TARGET_NAME     Boost::phoenix
	GIT_ADDR_HTTPS  "https://github.com/boostorg/phoenix.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/phoenix.git"
	DEPENDS assert
			bind
			config core
			function fusion
			mpl
			predef preprocessor proto
			range
			smart_ptr
			type_traits
			utility
)

simpleboost_add_sublib(poly_collection
	TARGET_NAME     Boost::poly_collection
	GIT_ADDR_HTTPS  "https://github.com/boostorg/poly_collection.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/poly_collection.git"
	DEPENDS assert
			config core
			iterator
			mp11 mpl
			type_erasure type_traits
)

simpleboost_add_sublib(polygon
	TARGET_NAME     Boost::polygon
	GIT_ADDR_HTTPS  "https://github.com/boostorg/polygon.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/polygon.git"
	DEPENDS config
)

simpleboost_add_sublib(pool
	TARGET_NAME     Boost::pool
	GIT_ADDR_HTTPS  "https://github.com/boostorg/pool.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/pool.git"
	DEPENDS assert
			config
			integer
			throw_exception type_traits
			winapi
)

simpleboost_add_sublib(predef
	TARGET_NAME     Boost::predef
	GIT_ADDR_HTTPS  "https://github.com/boostorg/predef.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/predef.git"
)

simpleboost_add_sublib(preprocessor
	TARGET_NAME     Boost::preprocessor
	GIT_ADDR_HTTPS  "https://github.com/boostorg/preprocessor.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/preprocessor.git"
)

simpleboost_add_sublib(process
	TARGET_NAME     Boost::process
	GIT_ADDR_HTTPS  "https://github.com/boostorg/process.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/process.git"
	DEPENDS algorithm asio
			config core
			filesystem fusion
			iterator
			move
			optional
			system
			tokenizer type_index
			winapi
)

simpleboost_add_sublib(program_options
	TARGET_NAME     Boost::program_options
	GIT_ADDR_HTTPS  "https://github.com/boostorg/program_options.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/program_options.git"
	DEPENDS any
			bind
			config core
			detail
			function
			iterator
			lexical_cast
			smart_ptr static_assert
			throw_exception tokenizer type_traits
)

simpleboost_add_sublib(property_map
	TARGET_NAME     Boost::property_map
	GIT_ADDR_HTTPS  "https://github.com/boostorg/property_map.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/property_map.git"
	DEPENDS any assert
			concept_check config core
			function
			iterator
			lexical_cast
			mpl
			smart_ptr static_assert
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(property_map_parallel
	TARGET_NAME     Boost::property_map_parallel
	GIT_ADDR_HTTPS  "https://github.com/boostorg/property_map_parallel.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/property_map_parallel.git"
	DEPENDS assert
			bind
			concept_check config
			function
			mpi mpl multi_index
			optional
			property_map
			serialization smart_ptr static_assert
			type_traits
)

simpleboost_add_sublib(property_tree
	TARGET_NAME     Boost::property_tree
	GIT_ADDR_HTTPS  "https://github.com/boostorg/property_tree.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/property_tree.git"
	DEPENDS any assert
			bind
			config core
			format
			iterator
			mpl multi_index
			optional
			range
			serialization static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(proto
	TARGET_NAME     Boost::proto
	GIT_ADDR_HTTPS  "https://github.com/boostorg/proto.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/proto.git"
	DEPENDS config core
			fusion
			mpl
			preprocessor
			range
			static_assert
			type_traits typeof
			utility
)

simpleboost_add_sublib(ptr_container
	TARGET_NAME     Boost::ptr_container
	GIT_ADDR_HTTPS  "https://github.com/boostorg/ptr_container.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/ptr_container.git"
	DEPENDS array assert
			circular_buffer config core
			iterator
			mpl
			range
			serialization smart_ptr static_assert
			type_traits
			unordered utility
)

simpleboost_add_sublib(python
	TARGET_NAME     Boost::python
	GIT_ADDR_HTTPS  "https://github.com/boostorg/python.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/python.git"
	DEPENDS align
			bind
			config conversion core
			detail
			foreach function
			graph
			integer iterator
			lexical_cast
			mpl
			numeric_conversion
			preprocessor property_map
			smart_ptr static_assert
			tuple type_traits
			utility
)

#   #######
#  ##     ##
#  ##     ##
#  ##     ##
#  ##  ## ##
#  ##    ##
#   ##### ##

simpleboost_add_sublib(qvm
	TARGET_NAME     Boost::qvm
	GIT_ADDR_HTTPS  "https://github.com/boostorg/qvm.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/qvm.git"
)

#  ########
#  ##     ##
#  ##     ##
#  ########
#  ##   ##
#  ##    ##
#  ##     ##

simpleboost_add_sublib(random
	TARGET_NAME     Boost::random
	GIT_ADDR_HTTPS  "https://github.com/boostorg/random.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/random.git"
	DEPENDS array assert
			config core
			dynamic_bitset
			integer io
			range
			static_assert system
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(range
	TARGET_NAME     Boost::range
	GIT_ADDR_HTTPS  "https://github.com/boostorg/range.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/range.git"
	DEPENDS array assert
			concept_check config container_hash conversion core
			detail
			iterator
			mpl
			optional
			preprocessor
			regex
			static_assert
			tuple type_traits
			utility
)

simpleboost_add_sublib(ratio
	TARGET_NAME     Boost::ratio
	GIT_ADDR_HTTPS  "https://github.com/boostorg/ratio.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/ratio.git"
	DEPENDS config core
			integer
			mpl
			rational
			static_assert
			type_traits
)

simpleboost_add_sublib(rational
	TARGET_NAME     Boost::rational
	GIT_ADDR_HTTPS  "https://github.com/boostorg/rational.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/rational.git"
	DEPENDS assert
			config core
			integer
			static_assert
			throw_exception type_traits
			utility
)

simpleboost_add_sublib(regex
	TARGET_NAME     Boost::regex
	GIT_ADDR_HTTPS  "https://github.com/boostorg/regex.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/regex.git"
	DEPENDS assert
			config
			predef
			throw_exception
)

simpleboost_add_sublib(regex_icu
	TARGET_NAME     Boost::regex_icu
	GIT_ADDR_HTTPS  "https://github.com/boostorg/regex.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/regex.git"
	DEPENDS assert
			config
			predef
			throw_exception
)

#   ######
#  ##    ##
#  ##
#   ######
#        ##
#  ##    ##
#   ######

simpleboost_add_sublib(safe_numerics
	TARGET_NAME     Boost::safe_numerics
	GIT_ADDR_HTTPS  "https://github.com/boostorg/safe_numerics.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/safe_numerics.git"
	DEPENDS concept_check config core
			integer
			logic
			mp11
)

simpleboost_add_sublib(scope_exit
	TARGET_NAME     Boost::scope_exit
	GIT_ADDR_HTTPS  "https://github.com/boostorg/scope_exit.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/scope_exit.git"
	DEPENDS config
			function
			preprocessor
			type_traits typeof
)

simpleboost_add_sublib(serialization
	TARGET_NAME     Boost::serialization
	GIT_ADDR_HTTPS  "https://github.com/boostorg/serialization.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/serialization.git"
	DEPENDS array assert
			config core
			detail
			function
			integer io iterator
			move mpl
			optional
			predef preprocessor
			smart_ptr spirit static_assert
			type_traits
			unordered utility
			variant
)

simpleboost_add_sublib(signals2
	TARGET_NAME     Boost::signals2
	GIT_ADDR_HTTPS  "https://github.com/boostorg/signals2.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/signals2.git"
	DEPENDS assert
			bind
			config core
			function
			iterator
			mpl
			optional
			parameter predef preprocessor
			smart_ptr
			throw_exception tuple type_traits
			variant
)

simpleboost_add_sublib(smart_ptr
	TARGET_NAME     Boost::smart_ptr
	GIT_ADDR_HTTPS  "https://github.com/boostorg/smart_ptr.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/smart_ptr.git"
	DEPENDS assert
			config core
			move
			static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(sort
	TARGET_NAME     Boost::sort
	GIT_ADDR_HTTPS  "https://github.com/boostorg/sort.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/sort.git"
	DEPENDS config core
			range
			static_assert
			type_traits
)

simpleboost_add_sublib(spirit
	TARGET_NAME     Boost::spirit
	GIT_ADDR_HTTPS  "https://github.com/boostorg/spirit.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/spirit.git"
	DEPENDS array assert
			config core
			endian
			function function_types fusion
			integer io iterator
			move mpl
			optional
			phoenix pool preprocessor proto
			range regex
			smart_ptr static_assert
			thread throw_exception type_traits typeof
			unordered utility
			variant
)

simpleboost_add_sublib(stacktrace
	TARGET_NAME     Boost::stacktrace
	GIT_ADDR_HTTPS  "https://github.com/boostorg/stacktrace.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/stacktrace.git"
	DEPENDS array
			config container_hash core
			predef
			static_assert
			type_traits
			winapi
)

simpleboost_add_sublib(statechart
	TARGET_NAME     Boost::statechart
	GIT_ADDR_HTTPS  "https://github.com/boostorg/statechart.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/statechart.git"
	DEPENDS assert
			bind
			config conversion core
			detail
			function
			mpl
			smart_ptr static_assert
			thread type_traits
)

simpleboost_add_sublib(static_assert
	TARGET_NAME     Boost::static_assert
	GIT_ADDR_HTTPS  "https://github.com/boostorg/static_assert.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/static_assert.git"
	DEPENDS config
)

simpleboost_add_sublib(static_string
	TARGET_NAME     Boost::static_string
	GIT_ADDR_HTTPS  "https://github.com/boostorg/static_string.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/static_string.git"
	DEPENDS assert
			container_hash
			static_assert
			throw_exception
			utility
)

simpleboost_add_sublib(stl_interfaces
	TARGET_NAME     Boost::stl_interfaces
	GIT_ADDR_HTTPS  "https://github.com/boostorg/stl_interfaces.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/stl_interfaces.git"
	DEPENDS assert
			config
)

simpleboost_add_sublib(system
	TARGET_NAME     Boost::system
	GIT_ADDR_HTTPS  "https://github.com/boostorg/system.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/system.git"
	DEPENDS assert
			config
			throw_exception
			variant2
			winapi
)

#  ########
#     ##
#     ##
#     ##
#     ##
#     ##
#     ##

simpleboost_add_sublib(test
	TARGET_NAME     Boost::test
	GIT_ADDR_HTTPS  "https://github.com/boostorg/test.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/test.git"
	DEPENDS algorithm assert
			bind
			config core
			detail
			exception
			function
			io iterator
			mpl
			numeric_conversion
			optional
			preprocessor
			smart_ptr static_assert
			type_traits
			utility
)

simpleboost_add_sublib(thread
	TARGET_NAME     Boost::thread
	GIT_ADDR_HTTPS  "https://github.com/boostorg/thread.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/thread.git"
	DEPENDS algorithm assert atomic
			bind
			chrono concept_check config container container_hash core
			date_time
			exception
			function
			intrusive io iterator
			lexical_cast
			move
			optional
			predef preprocessor
			smart_ptr static_assert system
			throw_exception tuple type_traits
			utility
			winapi
)

simpleboost_add_sublib(throw_exception
	TARGET_NAME     Boost::throw_exception
	GIT_ADDR_HTTPS  "https://github.com/boostorg/throw_exception.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/throw_exception.git"
	DEPENDS assert
			config
)

simpleboost_add_sublib(timer
	TARGET_NAME     Boost::timer
	GIT_ADDR_HTTPS  "https://github.com/boostorg/timer.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/timer.git"
	DEPENDS chrono config core
			io
			predef
			system
			throw_exception
)

simpleboost_add_sublib(tokenizer
	TARGET_NAME     Boost::tokenizer
	GIT_ADDR_HTTPS  "https://github.com/boostorg/tokenizer.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/tokenizer.git"
	DEPENDS assert
			config
			iterator
			mpl
			throw_exception type_traits
)

simpleboost_add_sublib(tti
	TARGET_NAME     Boost::tti
	GIT_ADDR_HTTPS  "https://github.com/boostorg/tti.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/tti.git"
	DEPENDS config
			function_types
			mpl
			preprocessor
			type_traits
)

simpleboost_add_sublib(tuple
	TARGET_NAME     Boost::tuple
	GIT_ADDR_HTTPS  "https://github.com/boostorg/tuple.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/tuple.git"
	DEPENDS config core
			static_assert type_traits
)

simpleboost_add_sublib(type_erasure
	TARGET_NAME     Boost::type_erasure
	GIT_ADDR_HTTPS  "https://github.com/boostorg/type_erasure.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/type_erasure.git"
	DEPENDS assert
			config core
			fusion
			iterator
			mp11 mpl
			preprocessor
			smart_ptr
			thread throw_exception type_traits typeof
			vmd
)

simpleboost_add_sublib(type_index
	TARGET_NAME     Boost::type_index
	GIT_ADDR_HTTPS  "https://github.com/boostorg/type_index.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/type_index.git"
	DEPENDS config container_hash core
			preprocessor
			smart_ptr static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(type_traits
	TARGET_NAME     Boost::type_traits
	GIT_ADDR_HTTPS  "https://github.com/boostorg/type_traits.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/type_traits.git"
	DEPENDS config
			static_assert
)

simpleboost_add_sublib(typeof
	TARGET_NAME     Boost::typeof
	GIT_ADDR_HTTPS  "https://github.com/boostorg/typeof.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/typeof.git"
	DEPENDS config
			preprocessor
			type_traits
)

#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#   #######

simpleboost_add_sublib(units
	TARGET_NAME     Boost::units
	GIT_ADDR_HTTPS  "https://github.com/boostorg/units.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/units.git"
	DEPENDS assert
			config core
			integer io
			lambda
			math mpl
			preprocessor
			static_assert
			type_traits typeof
)

simpleboost_add_sublib(unordered
	TARGET_NAME     Boost::unordered
	GIT_ADDR_HTTPS  "https://github.com/boostorg/unordered.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/unordered.git"
	DEPENDS assert
			config container container_hash core
			detail
			move
			predef preprocessor
			smart_ptr
			throw_exception tuple type_traits
)

simpleboost_add_sublib(utility
	TARGET_NAME     Boost::utility
	GIT_ADDR_HTTPS  "https://github.com/boostorg/utility.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/utility.git"
	DEPENDS config core
			io
			preprocessor
			static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(uuid
	TARGET_NAME     Boost::uuid
	GIT_ADDR_HTTPS  "https://github.com/boostorg/uuid.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/uuid.git"
	DEPENDS assert
			config container_hash core
			io
			move
			numeric_conversion
			predef
			random
			serialization static_assert
			throw_exception tti type_traits
			winapi
)

#  ##     ##
#  ##     ##
#  ##     ##
#  ##     ##
#   ##   ##
#    ## ##
#     ###

simpleboost_add_sublib(variant
	TARGET_NAME     Boost::variant
	GIT_ADDR_HTTPS  "https://github.com/boostorg/variant.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/variant.git"
	DEPENDS assert
			bind
			config container_hash core
			detail
			integer
			move mpl
			preprocessor
			static_assert
			throw_exception type_index type_traits
			utility
)

simpleboost_add_sublib(variant2
	TARGET_NAME     Boost::variant2
	GIT_ADDR_HTTPS  "https://github.com/boostorg/variant2.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/variant2.git"
	DEPENDS assert
			config
			mp11
)

simpleboost_add_sublib(vmd
	TARGET_NAME     Boost::vmd
	GIT_ADDR_HTTPS  "https://github.com/boostorg/vmd.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/vmd.git"
	DEPENDS preprocessor
)

#  ##      ##
#  ##  ##  ##
#  ##  ##  ##
#  ##  ##  ##
#  ##  ##  ##
#  ##  ##  ##
#   ###  ###

simpleboost_add_sublib(wave
	TARGET_NAME     Boost::wave
	GIT_ADDR_HTTPS  "https://github.com/boostorg/wave.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/wave.git"
	DEPENDS assert
			concept_check config core
			filesystem
			iterator
			lexical_cast
			mpl multi_index
			optional
			pool preprocessor
			serialization smart_ptr spirit static_assert
			throw_exception type_traits
)

simpleboost_add_sublib(winapi
	TARGET_NAME     Boost::winapi
	GIT_ADDR_HTTPS  "https://github.com/boostorg/winapi.git"
	GIT_ADDR_SSH    "git@github.com:boostorg/winapi.git"
	DEPENDS config
			predef
)
