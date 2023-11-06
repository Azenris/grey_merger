
cmake_minimum_required( VERSION 3.26.3...3.28 )

project(
	grey_merger
	VERSION 1.0
	LANGUAGES CXX
)

add_executable( grey_merger src/main.cpp )

target_include_directories( grey_merger PRIVATE "./third_party/" )

add_compile_definitions( grey_merger PRIVATE C_PLUS_PLUS )
add_compile_definitions( grey_merger PRIVATE _CRT_SECURE_NO_WARNINGS )
add_compile_definitions( grey_merger PRIVATE "$<$<CONFIG:Debug>:DEBUG>" )
add_compile_definitions( grey_merger PRIVATE "$<$<CONFIG:Release>:NDEBUG>" )

target_compile_features( grey_merger PRIVATE cxx_std_20 )

if ( MSVC )
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