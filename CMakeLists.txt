cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)

project(WestPatch VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory($ENV{HOME}/JUCE JUCE_build)

juce_add_plugin(WestPatch
    COMPANY_NAME "WestPatch"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE WPTC
    PLUGIN_CODE WPSM
    FORMATS AU
    PRODUCT_NAME "WestPatch"
)

juce_generate_juce_header(WestPatch)

# Plocka upp alla sourcefiler automatiskt
file(GLOB_RECURSE WESTPATCH_SOURCES
    CONFIGURE_DEPENDS
    Source/*.cpp
)

file(GLOB_RECURSE WESTPATCH_HEADERS
    CONFIGURE_DEPENDS
    Source/*.h
)

target_sources(WestPatch
    PRIVATE
        ${WESTPATCH_SOURCES}
        ${WESTPATCH_HEADERS}
)

target_include_directories(WestPatch
    PRIVATE
        Source
)
target_compile_definitions(WestPatch
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(WestPatch
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
