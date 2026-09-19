cmake_minimum_required(VERSION 3.31)

# In-game settings menu for the mods in this repository, rendered through every supported menu framework.

project(
	ModConfigUI
	LANGUAGES CXX
)

find_package(spdlog REQUIRED CONFIG)
find_package(imgui REQUIRED CONFIG)	# Required by the F.U.C.K API header

include(cmake/sourcelist.cmake)

add_library(ModConfigUI STATIC ${SOURCES})

target_include_directories(
	ModConfigUI
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/lib/SKSE-Menu-Framework-3-API"
		"${CMAKE_SOURCE_DIR}/lib"	# For "FUCK/src/FUCK_API.h" - the folder itself can't be added, its PCH.h would shadow ours
)

target_link_libraries(
	ModConfigUI
	PUBLIC
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog
	PRIVATE
		imgui::imgui
)

set_target_properties(ModConfigUI PROPERTIES FOLDER "Libs")

# Windows.h is deliberately not part of the PCH, CommonLib has to be included first.
target_precompile_headers(
	ModConfigUI
	PRIVATE
		"include/PCH.h"
)

target_compile_features(
	ModConfigUI
	PRIVATE
		cxx_std_23
)

target_compile_options(
	ModConfigUI
	PRIVATE
		/utf-8	# Set Source and Executable character sets to UTF-8
		/permissive-	# Standards conformance
		/W4	# Warning level

		/wd4100	# unreferenced formal parameter

		# Disable warnings from external libraries
		/external:anglebrackets
		/external:W0
)

source_group(
	TREE "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES ${SOURCES}
)
