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

find_package( OpenMP REQUIRED)
if(OPENMP_FOUND)
	message("OPENMP FOUND")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(Evergarden VAD/mrcg.cpp pcmio.cpp main.cpp VAD/bDNN.cpp VAD/prediction.cpp VAD/datafeeder.cpp)
#add_library(libEvergarden SHARED mrcg.cpp pcmio.cpp libevergarden.cpp bDNN.cpp prediction.cpp)

include_directories(${PROJECT_SOURCE_DIR}/deps/libtensorflow/include)

#include_directories(${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/include ${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/include_absl ${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/include_eigen ${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/include_eigen_archive ${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/include_pb ${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/include_proto)

target_link_libraries(Evergarden ${CONAN_LIBS} ${PROJECT_SOURCE_DIR}/deps/libtensorflow/lib/tensorflow.lib)
#target_link_libraries(libEvergarden ${CONAN_LIBS} ${PROJECT_SOURCE_DIR}/deps/libtensorflow_cc/lib/tensorflow_cc.lib)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(Evergarden stdc++fs)
    #target_link_libraries(libEvergarden stdc++fs)
endif()
