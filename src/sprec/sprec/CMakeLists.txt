cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(sprec CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(sprec_headers
  flac_encoder.h
  recognize.h
  sprec.h
  wav.h
  web_client.h
)

set(sprec_src
  flac_encoder.c
  recognize.c
  wav.c
  web_client.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -std=c99")

add_subdirectory(jsonz EXCLUDE_FROM_ALL)
include_directories(jsonz
                    ${CMAKE_BINARY_DIR}/jsonz/)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(FLAC REQUIRED)

pkg_check_modules(CURL libcurl)
pkg_check_modules(ALSA alsa)

include_directories(SYSTEM
	${CURL_INCLUDE_DIRS}
        ${FLAC_INCLUDE_DIRS}
        ${ALSA_INCLUDE_DIRS}
)

include(GenerateExportHeader)

add_library(sprec STATIC
  ${sprec_headers}
  ${sprec_src}
)

target_link_libraries(sprec
                      jsonz
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CURL_LIBRARIES}
                      ${FLAC_LIBRARIES}
                      ${ALSA_LIBRARIES}
)

generate_export_header(sprec)
add_compiler_export_flags()
