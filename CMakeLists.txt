cmake_minimum_required(VERSION 3.13)
project(Evergarden)

set(CMAKE_CXX_STANDARD 17)

include(conan_bootstrap.cmake)

conan_cmake_run(
        REQUIRES fmt/5.3.0@bincrafters/stable
        REQUIRES spdlog/1.3.1@bincrafters/stable
        BASIC_SETUP
        SETTINGS cppstd=17
        BUILD missing)

add_executable(Evergarden mrcg.cpp pcmio.cpp main.cpp)
add_library(libEvergarden SHARED mrcg.cpp pcmio.cpp libevergarden.cpp)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(Evergarden ${CONAN_LIBS} stdc++fs)
    target_link_libraries(libEvergarden ${CONAN_LIBS} stdc++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_link_libraries(Evergarden ${CONAN_LIBS})
    target_link_libraries(libEvergarden ${CONAN_LIBS})
endif()
