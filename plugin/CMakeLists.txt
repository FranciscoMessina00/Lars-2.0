cmake_minimum_required(VERSION 3.22)

# Version is needed by JUCE.
project(SlothPlugin VERSION 0.1.0)

# Adding a directory with the library/application name as a subfolder of the
# include folder is a good practice. It helps avoid name clashes later on.
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/SlothPlugin")
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libs/libtorch")
find_package(Torch REQUIRED)

# Adds a plugin target (that's basically what the Projucer does).
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME Sloth # change this
    IS_SYNTH FALSE # may change this
    NEEDS_MIDI_INPUT FALSE # may change this
    NEEDS_MIDI_OUTPUT FALSE # may change this
    PLUGIN_MANUFACTURER_CODE slth # change this
    PLUGIN_CODE slpg # change this
    FORMATS Standalone VST3 # may change this
    PRODUCT_NAME "SlothPlugin" # change this
)


# Sets the source files of the plugin project.
target_sources(${PROJECT_NAME}
    PRIVATE
        source/PluginEditor.cpp
        source/PluginProcessor.cpp
        source/WaveThumbnail.cpp
        source/Parameters.cpp
        ${INCLUDE_DIR}/PluginEditor.h
        ${INCLUDE_DIR}/PluginProcessor.h
        ${INCLUDE_DIR}/WaveThumbnail.h
        ${INCLUDE_DIR}/Parameters.h
)

# Sets the include directories of the plugin project.
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TORCH_INCLUDE_DIRS}
)

# Links to all necessary dependencies. The present ones are recommended by JUCE.
# If you use one of the additional modules, like the DSP module, you need to specify it here.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        torch
    PUBLIC
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        
)
# The following code block is suggested to be used on Windows.
# According to https://github.com/pytorch/pytorch/issues/25457,
# the DLLs need to be copied to avoid memory errors.
if (MSVC)
  file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
  add_custom_command(TARGET SlothPlugin_Standalone
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${TORCH_DLLS}
                     $<TARGET_FILE_DIR:SlothPlugin_Standalone>)
endif (MSVC)

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        # JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
)

# Enables all warnings and treats warnings as errors.
# This needs to be set up only for your projects, not 3rd party
# if (MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
# endif()







# In Visual Studio this command provides a nice grouping of source files in "filters".
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)
