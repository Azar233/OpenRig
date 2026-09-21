add_library(openrig_dsp STATIC
    src/GainNode.cpp
    src/SoftClipNode.cpp
)

add_library(OpenRig::DSP ALIAS openrig_dsp)

target_include_directories(openrig_dsp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(openrig_dsp
    PUBLIC
        OpenRig::Core
        openrig_project_options
    PRIVATE
        openrig_project_warnings
)
