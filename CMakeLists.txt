cmake_minimum_required(VERSION 3.23.1)

project(BetterPresser VERSION 0.0.1)

# General setup
set(PROJECT_NAME "BetterPresser")
set(CURRENT_VERSION "0.0.1")
set(PRODUCT_NAME "BetterPresser")
set(COMPANY_NAME "Archie DSP")
set(BUNDLE_ID "com.archiedsp.BetterPresser")
set(FORMATS Standalone VST3)

# JUCE
add_subdirectory(modules/JUCE)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows runtime library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif()

# Define the plugin target
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE ADSP
    PLUGIN_CODE SmGP
    FORMATS ${FORMATS}
    PRODUCT_NAME "${PRODUCT_NAME}"
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
)

# Source files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)

# Add sources directly to plugin target
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# Optional assets
file(GLOB_RECURSE Assets CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.*"
)

# Only create binary data if assets exist
if(Assets)
    juce_add_binary_data(AudioPluginData
        SOURCES ${Assets}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE AudioPluginData)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_BINARY_DIR}/AudioPluginData"
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_ENABLE_REPAINT_DEBUGGING=0
)

# Link JUCE modules
target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_product_unlocking
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# ==============================================================================
# Unit Test Suite Target
# ==============================================================================
add_executable(BetterPresserTests
    "${CMAKE_CURRENT_SOURCE_DIR}/Tests/TestMain.cpp"
    ${SourceFiles}
)

target_include_directories(BetterPresserTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

target_compile_definitions(BetterPresserTests PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JucePlugin_Name="BetterPresser"
    JucePlugin_IsMidiEffect=0
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
)

target_link_libraries(BetterPresserTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)