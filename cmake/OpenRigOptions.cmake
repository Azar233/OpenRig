option(OPENRIG_BUILD_TESTS "Build OpenRig tests" ON)
option(OPENRIG_BUILD_SANDBOX "Build the command-line DSP sandbox" ON)
option(OPENRIG_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(openrig_project_options INTERFACE)
target_compile_features(openrig_project_options INTERFACE cxx_std_20)

if(MSVC)
    target_compile_definitions(openrig_project_options INTERFACE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _USE_MATH_DEFINES
    )
endif()
