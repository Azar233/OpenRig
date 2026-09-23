option(OPENRIG_BUILD_TESTS "Build OpenRig tests" ON)
option(OPENRIG_BUILD_SANDBOX "Build the command-line DSP sandbox" ON)
option(OPENRIG_BUILD_IPLUG2 "Build the iPlug2 standalone application" OFF)
option(OPENRIG_IPLUG2_ENABLE_ASIO "Compile the Steinberg ASIO SDK path (requires a compatible license decision)" OFF)
option(OPENRIG_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(openrig_project_options INTERFACE)
target_compile_features(openrig_project_options INTERFACE cxx_std_20)

if(MSVC)
    target_compile_definitions(openrig_project_options INTERFACE
        NOMINMAX
        _USE_MATH_DEFINES
    )
endif()
