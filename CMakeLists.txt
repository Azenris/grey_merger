
cmake_minimum_required( VERSION 3.26.3 )

project(
	grey_merger
	VERSION 1.0
	LANGUAGES CXX
)

add_executable( grey_merger src/main.cpp )

set_target_properties(
	grey_merger
	PROPERTIES
	OUTPUT_NAME                      "grey_merger"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/builds/debug"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/builds/release"
)

target_include_directories( grey_merger PRIVATE "./third_party/" )

target_compile_definitions( grey_merger PRIVATE C_PLUS_PLUS )
target_compile_definitions( grey_merger PRIVATE "$<$<CONFIG:Debug>:DEBUG>" )
target_compile_definitions( grey_merger PRIVATE "$<$<CONFIG:Release>:NDEBUG>" )

target_compile_features( grey_merger PRIVATE cxx_std_20 )

if ( MSVC )
	target_compile_definitions( grey_merger PRIVATE _CRT_SECURE_NO_WARNINGS )
	target_compile_options( grey_merger PRIVATE -WX -W4 -wd4100 -wd4201 -wd4706 -Zc:preprocessor -Zc:strictStrings -GR- )
	target_compile_options( grey_merger PRIVATE $<$<CONFIG:Debug>:-Z7 -FC -MDd> )
	target_compile_options( grey_merger PRIVATE $<$<CONFIG:Release>:-MD -O2 -Ot -GF> )
endif()

if ( CMAKE_COMPILER_IS_GNUCC )
	target_compile_options( grey_merger PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-uninitialized -Wno-non-virtual-dtor -fno-rtti )
	target_compile_options( grey_merger PRIVATE $<$<CONFIG:Debug>:-OO -g> )
	target_compile_options( grey_merger PRIVATE $<$<CONFIG:Release>:-O2> )
endif()

if ( CMAKE_SYSTEM_NAME STREQUAL "Windows" )
	target_compile_definitions( grey_merger PRIVATE "PLATFORM_WINDOWS" )
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
	target_compile_definitions( grey_merger PRIVATE "PLATFORM_LINUX" )
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
	target_compile_definitions( grey_merger PRIVATE "PLATFORM_MAC" )
endif()