# Copyright (c) 2022 SimpleCMakeScripts
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


function(simplecmakescripts_print_module_path)
	message(
		STATUS
		"${SIMPLECMAKESCRIPTS_PROJECT_NAME} - "
		"CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
	)
endfunction(simplecmakescripts_print_module_path)


macro(simplecmakescripts_enable)
	message(
		STATUS
		"${SIMPLECMAKESCRIPTS_PROJECT_NAME} - "
		"Enabling in project ${PROJECT_NAME}..."
	)

	list(APPEND CMAKE_MODULE_PATH ${SIMPLECMAKESCRIPTS_SCRIPTS_DIR})

	simplecmakescripts_print_module_path()

endmacro(simplecmakescripts_enable)
