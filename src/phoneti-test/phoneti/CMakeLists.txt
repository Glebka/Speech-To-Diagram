cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

project(libphonetisaurus CXX C)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(library_sources
    FstPathFinder.cpp
    FstPathFinder.hpp
    MBRDecoder.cpp
    MBRDecoder.hpp
    Phonetisaurus.cpp
    Phonetisaurus.hpp
    util.cpp
    util.hpp
    cphonetisaurus.h
    cphonetisaurus.cpp
)

include(CheckCXXCompilerFlag)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  CHECK_CXX_COMPILER_FLAG("-std=c++11" STDCPP11_SUPPORTED)
  if(STDCPP11_SUPPORTED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" STDCPP0X_SUPPORTED)
    if(STDCPP0X_SUPPORTED)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
      message(FATAL_ERROR "Compiler doesn't appear to support C++11 or at least C++0x")
    endif()
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -ggdb3 -Wall -Wextra -pedantic")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
else()
  message(FATAL_ERROR "Unsupported compiler")
endif()

include_directories(3rdparty/sparsehash)
include_directories(3rdparty/utfcpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_directories(/usr/lib)

find_library(FST_LIB fst)
find_library(DL_LIB dl)
find_library(MATH_LIB m)
find_library(RT_LIB rt)

include(GenerateExportHeader)

add_library(phonetisaurus STATIC ${library_sources})
target_link_libraries(phonetisaurus
    ${FST_LIB}
    ${DL_LIB}
    ${MATH_LIB}
    ${RT_LIB}
)
generate_export_header(phonetisaurus)
add_compiler_export_flags()
