function(openrig_add_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target}
        PRIVATE
            OpenRig::Core
            OpenRig::DSP
            openrig_project_warnings
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

openrig_add_test(openrig_node_tests test_nodes.cpp)
openrig_add_test(openrig_graph_compiler_tests test_graph_compiler.cpp)
openrig_add_test(openrig_graph_execution_tests test_graph_execution.cpp)
openrig_add_test(openrig_parameter_transport_tests test_parameter_transport.cpp)
openrig_add_test(openrig_audio_engine_tests test_audio_engine.cpp)
