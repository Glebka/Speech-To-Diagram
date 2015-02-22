cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(gstspeechrecognition C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PkgConfig REQUIRED)
find_package(GStreamer REQUIRED)
find_package(GLIB COMPONENTS gobject REQUIRED)
find_package(JsonGlib REQUIRED)
find_package(CURL REQUIRED)
pkg_check_modules(SPHINX REQUIRED pocketsphinx)
pkg_check_modules(SPHINXBASE REQUIRED sphinxbase)

set(library_sources
    googmarshal.c
    googmarshal.h
    gstgooglespeech.c
    gstgooglespeech.h
    gstpocketsphinx.c
    gstpocketsphinx.h
    gstvader.c
    gstvader.h
    psmarshal.c
    psmarshal.h
)

include_directories(SYSTEM
    ${GLIB_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${SPHINXBASE_INCLUDE_DIRS}
    ${SPHINX_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

link_directories(
    ${GSTREAMER_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GLIB_GOBJECT_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
    ${SPHINXBASE_LIBRARIES}
    ${SPHINX_LIBRARIES}
    ${CURL_LIBRARIES}
)

add_library( gstspeechrecognition SHARED ${library_sources} )

target_link_libraries( gstspeechrecognition
    ${GSTREAMER_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GLIB_GOBJECT_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
    ${SPHINXBASE_LIBRARIES}
    ${SPHINX_LIBRARIES}
    ${CURL_LIBRARIES}
)
