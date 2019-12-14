cmake_minimum_required(VERSION 2.8)
project(base)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

#add_definitions(-DYAL_DOESNT_USE_TIMEZONE)

include_directories(
    ../../include
)

set(SOURCE_FILES
    ../../include/yal/dtf.hpp
    ../../include/yal/index.hpp
    ../../include/yal/options.hpp
    ../../include/yal/throw.hpp
    ../../include/yal/yal.hpp
    #
    main.cpp
    ../../src/index.cpp
    ../../src/yal.cpp
)

add_executable(base ${SOURCE_FILES})

target_link_libraries(
    base
    z
)
