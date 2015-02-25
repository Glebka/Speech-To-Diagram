cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(testp C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (sources
    main.c
)

add_subdirectory(phoneti EXCLUDE_FROM_ALL)
include_directories(phoneti)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/phoneti)

add_executable(testp
    ${sources}
)

target_link_libraries(testp phonetisaurus -lstdc++ )
