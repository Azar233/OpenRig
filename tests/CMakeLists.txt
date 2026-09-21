add_executable(openrig_core_tests
    test_main.cpp
)

target_link_libraries(openrig_core_tests
    PRIVATE
        OpenRig::Core
        OpenRig::DSP
        openrig_project_warnings
)

add_test(NAME openrig_core_tests COMMAND openrig_core_tests)
