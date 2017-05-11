cmake_minimum_required(VERSION 2.8)
project(base)

set(CMAKE_CXX_STANDARD 11)

include_directories(
    ../../include
)

set(SOURCE_FILES
    main.cpp
    ../../src/datetime.cpp
    ../../src/index.cpp
    ../../src/yal.cpp
)

link_directories(/usr/local/lib)

add_executable(base ${SOURCE_FILES})

target_link_libraries(base
    boost_system
    boost_filesystem
    z
)
