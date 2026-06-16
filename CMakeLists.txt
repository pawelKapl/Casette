cmake_minimum_required(VERSION 3.12.0)

set(NAME Casette)
project(${NAME} VERSION 0.0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(FFTCONVOLVER_SOURCES
    deps/FFTConvolver/FFTConvolver.cpp
    deps/FFTConvolver/AudioFFT.cpp
    deps/FFTConvolver/TwoStageFFTConvolver.cpp
    deps/FFTConvolver/Utilities.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${FFTCONVOLVER_SOURCES})

add_subdirectory(deps/NeuralAudio)
target_link_libraries(${PROJECT_NAME} PRIVATE NeuralAudio)

# Find JACK
find_package(PkgConfig REQUIRED)
pkg_check_modules(JACK REQUIRED jack)

# Find GPIOD
pkg_check_modules(GPIOD REQUIRED libgpiod)

# Find SNDFile
pkg_check_modules(SNDFILE REQUIRED sndfile)

# Find Aubio
pkg_check_modules(AUBIO REQUIRED aubio)

# libs
set(EIGEN_PATH deps/Eigen)
set(JSON_PATH deps/json)
set(PLOG_PATH deps/plog/include)
set(MATH_APPROX_PATH deps/math_approx/include)
set(FFT_CONVO_PATH deps/FFTConvolver)
set(LVGL_PATH deps)
set(LVGL_LINUX_PATH deps/lv_port_linux)
set(NEURAL_AUDIO_PATH deps/NeuralAudio/NeuralAudio)

include_directories(C:/Programowanie/pi/rpi-sysroot/usr/include)
include_directories(C:/Programowanie/pi/rpi-sysroot/usr/include/aarch64-linux-gnu)
include_directories(C:/Programowanie/pi/rpi-sysroot/usr/lib/aarch64-linux-gnu)
add_subdirectory(${LVGL_LINUX_PATH})


target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_NAME} PRIVATE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a72 -O3")

if (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${PROJECT_SOURCE_DIR}/src
      ${SNDFILE_INCLUDE_DIRS}
      ${AUBIO_INCLUDE_DIRS}
      ${NEURAL_AUDIO_PATH}
      ${JACK_INCLUDE_DIRS}
      ${MATH_APPROX_PATH}
      ${EIGEN_PATH}
      ${FFT_CONVO_PATH}
      ${LVGL_LINUX_PATH}
      ${JSON_PATH}
      ${PLOG_PATH}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JACK_LIBRARIES} ${GPIOD_LIBRARIES} ${SNDFILE_LIBRARIES} ${AUBIO_LIBRARIES} lvgl lvgl_linux)
endif()