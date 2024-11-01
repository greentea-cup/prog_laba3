cmake_minimum_required(VERSION 3.5.0)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${ProjectId} LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIERED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.c")
# note: if you want to change main target name (aka executable name)
# make sure you also change it in run.sh and run.bat
# set(MAIN_TARGET ${PROJECT_NAME})
set(MAIN_TARGET app)
add_executable(${MAIN_TARGET} ${PROJECT_SOURCE_FILES})

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${MAIN_TARGET} PRIVATE -Wall -Wextra -Wpedantic -O3 $<$<CONFIG:Debug>:-pg>)
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${MAIN_TARGET} PRIVATE -Wall -Wextra -Wpedantic -O3 $<$<CONFIG:Debug>:-pg>)
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	# message(FATAL_ERROR "NO GOD PLEASE NO! use clang or minGW if you are on windows")
	target_compile_options(${MAIN_TARGET} PRIVATE /utf-8 /J /W4 /Oi $<$<CONFIG:Release>:/O2>)
	target_compile_definitions(${MAIN_TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# target_link_libraries(${MAIN_TARGET} PRIVATE)
target_include_directories(${MAIN_TARGET} PRIVATE "include")

