###############################################################################
# Project
###############################################################################
cmake_minimum_required(VERSION 3.15)

# On M1 Macs, uncomment to enable universal binaries
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release")
project(rnbo_preset_issues VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

###############################################################################
# RNBO
###############################################################################
set(RNBO_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/export/rnbo/" CACHE FILEPATH "The path to the the RNBO c++ source directory")
set(RNBO_EXPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/export/" CACHE FILEPATH "path to your export directory")
set(RNBO_CLASS_FILE_NAME "rnbo_source.cpp" CACHE STRING "the name of your rnbo class file")
mark_as_advanced(RNBO_CLASS_FILE_NAME)

set(RNBO_CLASS_NAME "rnbomatic")
set(RNBO_CLASS_FILE "${RNBO_EXPORT_DIR}/${RNBO_CLASS_FILE_NAME}")
set(RNBO_DESCRIPTION_FILE "${RNBO_EXPORT_DIR}/description.json")
set(RNBO_PRESETS_FILE "${RNBO_EXPORT_DIR}/presets.json")
set(RNBO_BINARY_DATA_FILE "${RNBO_EXPORT_DIR}/${RNBO_CLASS_NAME}_binary.cpp")
set(RNBO_BINARY_DATA_STORAGE_NAME "${RNBO_CLASS_NAME}_binary")

include(${RNBO_CPP_DIR}/cmake/RNBODescriptionHeader.cmake)
set(DESCRIPTION_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
rnbo_write_description_header_if_exists(${RNBO_DESCRIPTION_FILE} ${DESCRIPTION_INCLUDE_DIR} ${RNBO_PRESETS_FILE})
include_directories(${DESCRIPTION_INCLUDE_DIR})

###############################################################################
# JUCE & Catch2
###############################################################################
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/vendor/catch2
  EXCLUDE_FROM_ALL
)

if (EXISTS ${RNBO_BINARY_DATA_FILE})
  add_definitions(-DRNBO_BINARY_DATA_STORAGE_NAME=${RNBO_BINARY_DATA_STORAGE_NAME})
endif()
#add_definitions(-DRNBO_JUCE_PARAM_DEFAULT_NOTIFY=1)

# Include the JUCE submodule, needed for JUCE-based CMake definitions
add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/vendor/juce
  ${CMAKE_BINARY_DIR}/juce
  EXCLUDE_FROM_ALL #don't build examples etc, also don't install
)

###############################################################################
# Tests
###############################################################################
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

file(GLOB_RECURSE PLUGIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
juce_add_console_app(rnbo_preset_issues_tests)
target_sources(rnbo_preset_issues_tests
  PRIVATE
  "${RNBO_CPP_DIR}/adapters/juce/RNBO_JuceAudioProcessor.cpp"
  "${RNBO_CPP_DIR}/adapters/juce/RNBO_JuceAudioProcessorEditor.cpp"
  "${RNBO_CPP_DIR}/RNBO.cpp"
  ${RNBO_CLASS_FILE}
  ${TEST_SOURCES}
  ${PLUGIN_SOURCES}
)

target_include_directories(rnbo_preset_issues_tests
  PRIVATE
  ${RNBO_CPP_DIR}/
  ${RNBO_CPP_DIR}/src
  ${RNBO_CPP_DIR}/common/
  ${RNBO_CPP_DIR}/adapters/juce/
  ${RNBO_CPP_DIR}/src/3rdparty/
  src
)
target_compile_definitions(rnbo_preset_issues_tests
  PUBLIC
  RNBO_JUCE_NO_CREATE_PLUGIN_FILTER=1
  DONT_SET_USING_JUCE_NAMESPACE=1
)
target_link_libraries(rnbo_preset_issues_tests
  PRIVATE
  Catch2::Catch2WithMain
  juce::juce_audio_utils
  PUBLIC
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)
juce_generate_juce_header(rnbo_preset_issues_tests)
