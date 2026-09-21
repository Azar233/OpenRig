cmake_minimum_required(VERSION 3.24)

project(
    OpenRig
    VERSION 0.1.0
    DESCRIPTION "A modular, real-time guitar DSP platform"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(OpenRigOptions)
include(OpenRigWarnings)

add_subdirectory(core)
add_subdirectory(dsp)

if(OPENRIG_BUILD_SANDBOX)
    add_subdirectory(apps/standalone)
endif()

if(OPENRIG_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
