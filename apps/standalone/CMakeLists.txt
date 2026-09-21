# Temporary command-line host. It verifies that Core and DSP stay independent from iPlug2.
# Replace/augment this target with the iPlug2OOS standalone adapter in T002/T003.
add_executable(openrig_sandbox main.cpp)
target_link_libraries(openrig_sandbox PRIVATE OpenRig::Core OpenRig::DSP openrig_project_warnings)
