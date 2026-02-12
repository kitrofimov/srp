# Software Rendering Pipeline (SRP) library
# Licensed under GNU GPLv3

cmake_minimum_required(VERSION 3.18.4)
project(srp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -std=c2x -march=native")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

if (ENABLE_ASAN)
	add_compile_options(-fsanitize=address)
	link_libraries(asan)
endif()
if (ENABLE_LSAN)
	add_compile_options(-fsanitize=leak)
	link_libraries(asan)
endif()
if (ENABLE_UBSAN)
	add_compile_options(-fsanitize=undefined)
	link_libraries(ubsan)
endif()
if (ENABLE_GPROF)
	add_compile_options(-pg)
	add_link_options(-pg)
endif()

add_subdirectory(src)
if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
if (BUILD_DOCS)
	add_subdirectory(docs)
endif()
