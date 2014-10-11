cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(jsonz CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(jsonz_headers
  jsonz.h
  object.h
)

set(jsonz_src
  jsonz.c
  object.c
)

include(GenerateExportHeader)
add_library(jsonz STATIC
  ${jsonz_headers}
  ${jsonz_src}
)
generate_export_header(jsonz)
add_compiler_export_flags()
