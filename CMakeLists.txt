cmake_minimum_required(VERSION 3.13)
project(Evergarden)

set(CMAKE_CXX_STANDARD 17)

include(conan_bootstrap.cmake)
include(tensorflow_bootstrap.cmake)

conan_cmake_run(
        REQUIRES fmt/5.3.0@bincrafters/stable
        REQUIRES spdlog/1.3.1@bincrafters/stable
        BASIC_SETUP
        SETTINGS cppstd=17
        BUILD missing)

add_executable(Evergarden mrcg.cpp pcmio.cpp main.cpp bDNN.cpp prediction.cpp)
add_library(libEvergarden SHARED mrcg.cpp pcmio.cpp libevergarden.cpp prediction.cpp)

include_directories(deps/libtensorflow/include)

target_link_libraries(Evergarden ${CONAN_LIBS} deps/libtensorflow/tensorflow.lib)
target_link_libraries(libEvergarden ${CONAN_LIBS} deps/libtensorflow/tensorflow.lib)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(Evergarden stdc++fs)
    target_link_libraries(libEvergarden stdc++fs)
endif()
