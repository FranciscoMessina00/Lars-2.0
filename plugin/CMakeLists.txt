cmake_minimum_required(VERSION 3.22)
project(SlothPlugin VERSION 0.1.0)

# --- Project Setup ---
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/SlothPlugin")

# --- LibTorch Setup ---
# Set path to the *directory containing* the libtorch folder
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libs/libtorch") # Assuming libtorch is in ../libs/libtorch
# Or directly: set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libs/libtorch") if FindTorch expects that

find_package(Torch REQUIRED)

# Add status messages to verify LibTorch detection
message(STATUS "TORCH_INSTALL_PREFIX = ${TORCH_INSTALL_PREFIX}") # Check CMake output for this line!
message(STATUS "TORCH_INCLUDE_DIRS = ${TORCH_INCLUDE_DIRS}")   # Check CMake output for this line!
message(STATUS "TORCH_LIBRARIES = ${TORCH_LIBRARIES}")       # Check CMake output for this line!

# Check if the critical lib directory exists
if(IS_DIRECTORY "${TORCH_INSTALL_PREFIX}/lib")
    message(STATUS "Verified LibTorch lib directory exists: ${TORCH_INSTALL_PREFIX}/lib")
else()
    message(FATAL_ERROR "LibTorch lib directory NOT found: ${TORCH_INSTALL_PREFIX}/lib - Check CMAKE_PREFIX_PATH and your libtorch installation.")
endif()

# --- JUCE Plugin Target ---
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME Sloth # change this
    IS_SYNTH FALSE # may change this
    NEEDS_MIDI_INPUT FALSE # may change this
    NEEDS_MIDI_OUTPUT FALSE # may change this
    PLUGIN_MANUFACTURER_CODE slth # change this
    PLUGIN_CODE slpg # change this
    FORMATS Standalone VST3 # may change this
    PRODUCT_NAME "Lars 2_0" # change this
    # VST3_AUTO_MANIFEST TRUE # Keep this if you were using it before, usually default
)

# --- Sources ---
target_sources(${PROJECT_NAME}
    PRIVATE
        source/PluginEditor.cpp
        source/PluginProcessor.cpp
        source/WaveThumbnail.cpp
        source/Parameters.cpp
        source/BufferAudioSource.cpp
        source/TransportComponent.cpp
        source/TransportOriginal.cpp
        source/TransportSeparation.cpp
        source/RoundedCornersEffect.cpp
        ${INCLUDE_DIR}/PluginEditor.h
        ${INCLUDE_DIR}/PluginProcessor.h
        ${INCLUDE_DIR}/WaveThumbnail.h
        ${INCLUDE_DIR}/BufferAudioSource.h
        ${INCLUDE_DIR}/Parameters.h
        ${INCLUDE_DIR}/TransportComponent.h
        ${INCLUDE_DIR}/TransportOriginal.h
        ${INCLUDE_DIR}/TransportSeparation.h
        ${INCLUDE_DIR}/RoundedCornersEffect.h
)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TORCH_INCLUDE_DIRS} # Include directories provided by find_package(Torch)
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)
set(IMAGES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../images")
juce_add_binary_data(${PROJECT_NAME}_data
     SOURCES
     #   You can add more files here if needed, e.g.:
     #   assets/background.png NAMESPACE GuiAssets
     ${IMAGES_SOURCE_DIR}/Crash.png
     ${IMAGES_SOURCE_DIR}/Hihat.png
     ${IMAGES_SOURCE_DIR}/Kick.png
     ${IMAGES_SOURCE_DIR}/Toms.png
     ${IMAGES_SOURCE_DIR}/Ride.png
     ${IMAGES_SOURCE_DIR}/Snare.png
     ${IMAGES_SOURCE_DIR}/divide_image.png
     ${IMAGES_SOURCE_DIR}/download_button.svg
     ${IMAGES_SOURCE_DIR}/pause_button.svg
     ${IMAGES_SOURCE_DIR}/play_button.svg
     ${IMAGES_SOURCE_DIR}/upload_button.svg
)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME}
    PUBLIC # JUCE modules are often needed publicly by the plugin wrapper
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats # Needed for plugin client wrappers
        juce::juce_audio_plugin_client # Needed for plugin client wrappers
        juce::juce_audio_processors # Needed for plugin client wrappers
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events # Often needed by gui/core
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        # juce::juce_recommended_lto_flags # LTO can sometimes cause issues, test with/without
        juce::juce_recommended_warning_flags

    PRIVATE # LibTorch likely only needed privately by your implementation
        # Use the imported target if find_package(Torch) provides it (preferred)
        # Common names: Torch::Torch or torch::torch. Check CMake output/docs.
        # Torch::Torch
        # If Torch::Torch doesn't work or isn't defined, fall back to the variable:
        # ${TORCH_LIBRARIES}
        # Or the simple name if that's how find_package defines it:
        torch
        ${PROJECT_NAME}_data
)






# --- Compile Definitions ---
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_USE_HIGHDPI=1
        # JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
)

# --- Post-Build DLL Copying for VST3 on Windows (MSVC) ---
# This addresses the issue where juce_vst3_helper.exe fails because
# it can't find the LibTorch DLL dependencies when loading the built VST3.

# Check if we are using MSVC (you can change back to TARGET check later)
if (MSVC) # Using simplified condition for testing
    message(STATUS "Setting up post-build DLL copy for targets and Helper.")

    # Find all DLLs in the LibTorch lib directory
    file(GLOB TORCH_DLLS CONFIGURE_DEPENDS "${TORCH_INSTALL_PREFIX}/lib/*.dll")

    if(NOT TORCH_DLLS)
        message(WARNING "Could not find any LibTorch DLLs in ${TORCH_INSTALL_PREFIX}/lib/. Post-build copy will not run.")
    else()
        message(STATUS "Found LibTorch DLLs to copy: ${TORCH_DLLS}")

        # Define target names (Verify these in VS Solution Explorer)
        set(VST3_TARGET_NAME "${PROJECT_NAME}_VST3")
        set(STANDALONE_TARGET_NAME "${PROJECT_NAME}_Standalone")
        set(MODEL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../models")

        if(TARGET ${VST3_TARGET_NAME})
            message(STATUS "VST3 target found: ${VST3_TARGET_NAME}")
            set(VST3_HELPER_TARGET_NAME "juce_vst3_helper")

            # Define destination directories using generator expressions
            set(VST3_DLL_DIR "$<TARGET_FILE_DIR:${VST3_TARGET_NAME}>")
            set(VST3_HELPER_DIR "$<TARGET_FILE_DIR:${VST3_HELPER_TARGET_NAME}>")

            message(STATUS "VST3 DLL Destination Dir (Generator Expression): ${VST3_DLL_DIR}")
            message(STATUS "VST3 Helper Dir (Generator Expression): ${VST3_HELPER_DIR}")

            # --- COMMAND 1: Attached to juce_vst3_helper ---
            # Copies DLLs next to juce_vst3_helper.exe immediately after it's built.
            # Hypothesis: This makes them available when the VST3 post-build runs the helper.
            add_custom_command(
                TARGET ${VST3_HELPER_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TORCH_DLLS}
                    ${VST3_HELPER_DIR}
                COMMENT "Copying LibTorch DLLs to Helper dir (${VST3_HELPER_DIR}) immediately after helper build"
                VERBATIM
            )

            # --- COMMAND 2: Attached to SlothPlugin_VST3 ---
            # Copies DLLs next to the final VST3 DLL after the VST3 target is built.
            # This ensures they are correctly placed for deployment/runtime use.
            add_custom_command(
                TARGET ${VST3_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TORCH_DLLS}
                    ${VST3_DLL_DIR}
                COMMENT "Copying LibTorch DLLs to final VST3 dir (${VST3_DLL_DIR}) after VST3 build"
                VERBATIM
            )

            set(MODEL_DEST_DIR "$<TARGET_FILE_DIR:${VST3_TARGET_NAME}>")

            # List all the files you want to copy:
            set(MODEL_FILES
                "${MODEL_SOURCE_DIR}/mdx23c.pt"
                "${MODEL_SOURCE_DIR}/mdx23c_capstone.pt"
                "${MODEL_SOURCE_DIR}/mdx23c_inst_sep.pt"
            )

            # Verify they exist (optional, but helpful)
            foreach(f IN LISTS MODEL_FILES)
              if(NOT EXISTS "${f}")
                message(WARNING "Model file not found: ${f}")
              endif()
            endforeach()

            add_custom_command(
              TARGET ${VST3_TARGET_NAME} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MODEL_FILES}        # all of your source files
                ${MODEL_DEST_DIR}      # destination directory
              COMMENT "Copying model files to VST3 output directory"
              VERBATIM
            )
        endif()
        if(TARGET ${STANDALONE_TARGET_NAME})
            message(STATUS "Standalone target found: ${STANDALONE_TARGET_NAME}")
            # Define destination directories using generator expressions
            set(STANDALONE_DLL_DIR "$<TARGET_FILE_DIR:${STANDALONE_TARGET_NAME}>")

            message(STATUS "STANDALONE DLL Destination Dir (Generator Expression): ${STANDALONE_DLL_DIR}")

            # --- COMMAND 1: Attached to SlothPlugin_Standalone ---
            # Copies DLLs next to the final VST3 DLL after the VST3 target is built.
            # This ensures they are correctly placed for deployment/runtime use.
            add_custom_command(
                TARGET ${STANDALONE_TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${TORCH_DLLS}
                    ${STANDALONE_DLL_DIR}
                COMMENT "Copying LibTorch DLLs to final Standalone dir (${STANDALONE_DLL_DIR}) after Standalone build"
                VERBATIM
            )

            set(MODEL_DEST_DIR "$<TARGET_FILE_DIR:${STANDALONE_TARGET_NAME}>")

            # List all the files you want to copy:
            set(MODEL_FILES
                "${MODEL_SOURCE_DIR}/mdx23c.pt"
                "${MODEL_SOURCE_DIR}/mdx23c_capstone.pt"
                "${MODEL_SOURCE_DIR}/mdx23c_inst_sep.pt"
            )

            # Verify they exist (optional, but helpful)
            foreach(f IN LISTS MODEL_FILES)
              if(NOT EXISTS "${f}")
                message(WARNING "Model file not found: ${f}")
              endif()
            endforeach()

            add_custom_command(
              TARGET ${STANDALONE_TARGET_NAME} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${MODEL_FILES}        # all of your source files
                ${MODEL_DEST_DIR}      # destination directory
              COMMENT "Copying model files to VST3 output directory"
              VERBATIM
            )
        endif()
        
    endif()
# else() block could go here if using the TARGET check
#    if(MSVC)
#         message(STATUS "Condition NOT MET: Skipping VST3 DLL copy setup...")
#    endif()
endif()

# --- Optional: Compiler Warnings ---
# if (MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
# else()
#     target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
# endif()

# --- Optional: Source Grouping for IDEs ---
# In Visual Studio this command provides a nice grouping of source files in "filters".
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)