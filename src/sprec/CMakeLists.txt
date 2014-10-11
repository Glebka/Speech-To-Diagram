cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(sprec_test CXX C)

set(project_sources
  simple.c
)

add_subdirectory(sprec EXCLUDE_FROM_ALL)
include_directories(sprec
                    ${CMAKE_BINARY_DIR}/sprec/)
                    
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
  "Choose the type of build: Debug/Release" FORCE)
endif()

message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

add_executable(sprec_test
               ${project_sources}
)

target_link_libraries(sprec_test
                      sprec
)