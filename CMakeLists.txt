cmake_minimum_required(VERSION 3.22)

project(pbTechLab_PrimitiveFxLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if (WIN32 AND NOT JUCE_WEBVIEW2_PACKAGE_LOCATION)
  set(PB_WEBVIEW2_PACKAGE_VERSION "1.0.1901.177" CACHE STRING "Microsoft.Web.WebView2 NuGet package version")
  set(PB_WEBVIEW2_NUGET_ROOT "$ENV{USERPROFILE}/AppData/Local/PackageManagement/NuGet/Packages" CACHE PATH "NuGet package root")
  set(PB_WEBVIEW2_PACKAGE_SOURCE "${PB_WEBVIEW2_NUGET_ROOT}/Microsoft.Web.WebView2.${PB_WEBVIEW2_PACKAGE_VERSION}")

  if (EXISTS "${PB_WEBVIEW2_PACKAGE_SOURCE}/build/native/include/WebView2.h")
    set(PB_WEBVIEW2_SEARCH_ROOT "${CMAKE_BINARY_DIR}/pb_webview2_package_root")
    set(PB_WEBVIEW2_PACKAGE_LINK "${PB_WEBVIEW2_SEARCH_ROOT}/Microsoft.Web.WebView2.${PB_WEBVIEW2_PACKAGE_VERSION}")
    file(MAKE_DIRECTORY "${PB_WEBVIEW2_SEARCH_ROOT}")

    if (NOT EXISTS "${PB_WEBVIEW2_PACKAGE_LINK}")
      file(CREATE_LINK "${PB_WEBVIEW2_PACKAGE_SOURCE}" "${PB_WEBVIEW2_PACKAGE_LINK}" SYMBOLIC COPY_ON_ERROR)
    endif()

    set(JUCE_WEBVIEW2_PACKAGE_LOCATION "${PB_WEBVIEW2_SEARCH_ROOT}" CACHE PATH "Single WebView2 NuGet package root" FORCE)
  endif()
endif()

option(PB_ENABLE_AAX "Build AAX plug-in targets" OFF)
option(PB_BUILD_STANDALONE "Build standalone plug-in targets" ON)

include(FetchContent)

set(PB_JUCE_LOCAL "" CACHE PATH "Optional local JUCE 8 checkout")
if (EXISTS "${PB_JUCE_LOCAL}/CMakeLists.txt")
  add_subdirectory("${PB_JUCE_LOCAL}" "${CMAKE_BINARY_DIR}/JUCE")
else()
  FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(JUCE)
endif()

set(PB_AAX_SDK_PATH "" CACHE PATH "Avid AAX SDK path")
if (PB_ENABLE_AAX)
  if (NOT PB_AAX_SDK_PATH AND DEFINED JUCE_AAX_SDK_PATH)
    set(PB_AAX_SDK_PATH "${JUCE_AAX_SDK_PATH}")
  endif()

  if (NOT PB_AAX_SDK_PATH)
    message(FATAL_ERROR "PB_ENABLE_AAX is ON, but PB_AAX_SDK_PATH or JUCE_AAX_SDK_PATH was not provided")
  endif()

  juce_set_aax_sdk_path("${PB_AAX_SDK_PATH}")
endif()

set(PB_PLUGIN_FORMATS VST3)
if (PB_ENABLE_AAX)
  list(APPEND PB_PLUGIN_FORMATS AAX)
endif()
if (PB_BUILD_STANDALONE)
  list(APPEND PB_PLUGIN_FORMATS Standalone)
endif()

function(pb_add_primitive_fx_plugin target_name product_name plugin_code plugin_kind plugin_desc mock_dir)
  juce_add_plugin(${target_name}
    COMPANY_NAME "pbTechLab"
    COMPANY_WEBSITE "https://pbtechlab.com/"
    BUNDLE_ID "com.pbtechlab.${target_name}"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEB_BROWSER TRUE
    NEEDS_WEBVIEW2 TRUE
    PLUGIN_MANUFACTURER_CODE PbTL
    PLUGIN_CODE ${plugin_code}
    AAX_CATEGORY AAX_ePlugInCategory_Effect
    FORMATS ${PB_PLUGIN_FORMATS}
    PRODUCT_NAME "${product_name}"
  )

  juce_add_binary_data(${target_name}_WebUI
    SOURCES
      "${mock_dir}/index.html"
      "${mock_dir}/style.css"
      "${mock_dir}/script.js"
  )

  target_sources(${target_name}
    PRIVATE
      "${PROJECT_SOURCE_DIR}/Common/PrimitiveFxProcessor.cpp"
      "${PROJECT_SOURCE_DIR}/Common/PrimitiveFxProcessor.h"
      "${PROJECT_SOURCE_DIR}/Common/PrimitiveFxEditor.cpp"
      "${PROJECT_SOURCE_DIR}/Common/PrimitiveFxEditor.h"
  )

  target_include_directories(${target_name} PRIVATE "${PROJECT_SOURCE_DIR}/Common")

  target_compile_definitions(${target_name}
    PUBLIC
      JUCE_WEB_BROWSER=1
      JUCE_USE_WIN_WEBVIEW2=1
      JUCE_VST3_CAN_REPLACE_VST2=0
    PRIVATE
      PB_PLUGIN_NAME="${product_name}"
      PB_PLUGIN_KIND="${plugin_kind}"
      PB_PLUGIN_DESCRIPTION="${plugin_desc}"
      PB_MOCK_DIR="${mock_dir}"
  )

  target_compile_options(${target_name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

  target_link_libraries(${target_name}
    PRIVATE
      juce::juce_audio_utils
      juce::juce_audio_processors
      juce::juce_dsp
      juce::juce_gui_extra
      ${target_name}_WebUI
    PUBLIC
      juce::juce_recommended_config_flags
      juce::juce_recommended_lto_flags
      juce::juce_recommended_warning_flags
  )
endfunction()

add_subdirectory(pbPFL_Reverb)
add_subdirectory(pbPFL_Delay)
add_subdirectory(pbPFL_Distortion)
add_subdirectory(pbPFL_Compressor)
add_subdirectory(pbPFL_Limiter)
add_subdirectory(pbPFL_3BandEQ)
add_subdirectory(pbPFL_4BandEQ)
add_subdirectory(pbPFL_PitchShifter)
add_subdirectory(pbPFL_Chorus)
add_subdirectory(pbPFL_Phaser)
add_subdirectory(pbPFL_Flanger)
add_subdirectory(pbPFL_StereoEnhancer)
add_subdirectory(pbPFL_StereoWidth)
add_subdirectory(pbPFL_MidSideProcessor)
add_subdirectory(pbPFL_AutoPan)
add_subdirectory(pbPFL_Tremolo)
add_subdirectory(pbPFL_Vibrato)
