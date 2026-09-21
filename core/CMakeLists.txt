add_library(openrig_core STATIC
    src/CompiledGraph.cpp
    src/GraphCompiler.cpp
    src/NodeRegistry.cpp
)

add_library(OpenRig::Core ALIAS openrig_core)

target_include_directories(openrig_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(openrig_core
    PUBLIC
        openrig_project_options
    PRIVATE
        openrig_project_warnings
)
