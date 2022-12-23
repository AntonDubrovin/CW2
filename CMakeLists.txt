cmake_minimum_required(VERSION 3.24)
project(CW2)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(Parlay REQUIRED)

include(FetchContent)
FetchContent_Declare(parlaylib
        GIT_REPOSITORY  https://github.com/cmuparlay/parlaylib.git
        GIT_TAG         master
        )
FetchContent_GetProperties(parlaylib)


if(NOT parlaylib_POPULATED)
    FetchContent_Populate(parlaylib)
    add_subdirectory(${parlaylib_SOURCE_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(CW2 bfs.cpp)

target_link_libraries(CW2 PUBLIC Parlay::parlay)

target_link_libraries(CW2 INTERFACE parlay)
