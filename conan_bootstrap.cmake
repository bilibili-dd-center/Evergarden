if(NOT EXISTS "${PROJECT_SOURCE_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${PROJECT_SOURCE_DIR}/conan.cmake")
endif()

include(${PROJECT_SOURCE_DIR}/conan.cmake)