cmake_minimum_required(VERSION 3.12.0)
project(octaver_render CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(octaver_render
    main.cpp
    ${ROOT}/src/dsp/octaver/octaver.cpp
)

target_include_directories(octaver_render PRIVATE ${ROOT}/src ${ROOT}/deps/json)

# same as the pedal build
set_source_files_properties(${ROOT}/src/dsp/octaver/octaver.cpp PROPERTIES COMPILE_OPTIONS "-fno-math-errno")

# MinGW hides M_PI unless this is set before the first <cmath>
if (WIN32)
    target_compile_definitions(octaver_render PRIVATE _USE_MATH_DEFINES=)
endif()

# same flags as the pedal build, so the timing matches
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    target_compile_options(octaver_render PRIVATE -mcpu=cortex-a72 -O3)
else()
    target_compile_options(octaver_render PRIVATE -O3)
endif()
