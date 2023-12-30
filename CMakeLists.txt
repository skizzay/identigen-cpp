cmake_minimum_required(VERSION 3.26)
project(identigen
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TESTS "Enable tests" ON)

include(FetchContent)

add_subdirectory(src/main/cpp)

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(src/test/cpp)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)