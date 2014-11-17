cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(sphinx CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST gstreamer-0.10)
pkg_check_modules(SPHINX pocketsphinx)


# Project directory with different statistical models for pocketsphinx
set(PROJECT_LANG_DATA_DIR ${CMAKE_SOURCE_DIR}/../../data)

# DEFAULT_ACOUSTIC_DATA_DIR - default directory with acoustic data preinstalled with pocketsphinx
# For example: /usr/share/pocketsphinx/model
execute_process(COMMAND pkg-config --variable=modeldir pocketsphinx OUTPUT_VARIABLE DEFAULT_ACOUSTIC_DATA_DIR)
STRING(REGEX REPLACE "(\r?\n)+$" "" DEFAULT_ACOUSTIC_DATA_DIR "${DEFAULT_ACOUSTIC_DATA_DIR}")

# Project directory inside the PROJECT_LANG_DATA_DIR with statistical language model
#set(PROJECT_LM_DIR_NAME default)
set(PROJECT_LM_DIR_NAME current)

#set(PROJECT_LM_FILE_EXTENSION .DMP)
set(PROJECT_LM_FILE_EXTENSION .lm)

# Dierctory with concrete acoustic model
set(PROJECT_ACOUSTIC_DATA_DIR ${DEFAULT_ACOUSTIC_DATA_DIR}/hmm/en_US/hub4wsj_sc_8k)

# Audio source gstreamer plugin
set(AUDIO_SOURCE_PLUGIN alsasrc)

set(project_headers

    CMainLoop.hpp
    CGstElement.hpp
    CGstSource.hpp
    CGstAudioConverter.hpp
    CGstAudioResampler.hpp
    CGstVoiceActivityDetector.hpp
    CGstSpeechRecognizer.hpp

)

set(project_sources

    CMainLoop.cpp
    CGstElement.cpp
    CGstSource.cpp
    CGstAudioConverter.cpp
    CGstAudioResampler.cpp
    CGstVoiceActivityDetector.cpp
    CGstSpeechRecognizer.cpp
    main.cpp

)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
  "Choose the type of build: Debug/Release" FORCE)
endif()

include_directories(SYSTEM
    ${GST_INCLUDE_DIRS}
)
link_directories(${GST_LIBRARY_DIRS})

message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

configure_file(config.h.template ${CMAKE_BINARY_DIR}/config.h)

add_executable(sphinx
	${project_sources}
	${project_headers}
)

target_link_libraries(sphinx
        ${GST_LIBRARIES}
)
