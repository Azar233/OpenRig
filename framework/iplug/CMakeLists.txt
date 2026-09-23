add_library(openrig_iplug_adapter STATIC
    src/IPlugAudioAdapter.cpp
)

add_library(OpenRig::IPlugAdapter ALIAS openrig_iplug_adapter)

target_include_directories(openrig_iplug_adapter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(openrig_iplug_adapter
    PUBLIC
        OpenRig::Core
        openrig_project_options
    PRIVATE
        openrig_project_warnings
)

if(OPENRIG_BUILD_IPLUG2)
    set(OPENRIG_IPLUG2_DIR
        "${PROJECT_SOURCE_DIR}/third_party/iPlug2OOS/iPlug2"
        CACHE PATH
        "Path to the pinned iPlug2 framework checkout"
    )

    if(NOT EXISTS "${OPENRIG_IPLUG2_DIR}/iPlug2.cmake")
        message(FATAL_ERROR
            "Pinned iPlug2 sources are missing. Run: "
            "git submodule update --init --recursive third_party/iPlug2OOS"
        )
    endif()

    set(IPLUG2_DIR "${OPENRIG_IPLUG2_DIR}" CACHE PATH "iPlug2 framework root" FORCE)
    set(IPLUG2_CXX_STANDARD 20 CACHE STRING "iPlug2 C++ standard" FORCE)
    include("${IPLUG2_DIR}/iPlug2.cmake")
    find_package(iPlug2 REQUIRED)

    if(WIN32 AND NOT OPENRIG_IPLUG2_ENABLE_ASIO)
        get_target_property(_openrig_app_sources iPlug2::APP INTERFACE_SOURCES)
        list(FILTER _openrig_app_sources EXCLUDE REGEX "[/\\\\](asio|asiodrivers|asiolist|iasiothiscallresolver)\\.cpp$")
        set_property(TARGET iPlug2::APP PROPERTY INTERFACE_SOURCES "${_openrig_app_sources}")

        get_target_property(_openrig_app_definitions iPlug2::APP INTERFACE_COMPILE_DEFINITIONS)
        list(REMOVE_ITEM _openrig_app_definitions __WINDOWS_ASIO__)
        set_property(TARGET iPlug2::APP PROPERTY INTERFACE_COMPILE_DEFINITIONS "${_openrig_app_definitions}")
        message(STATUS "OpenRig: ASIO backend disabled pending an explicit license decision")
    endif()

    add_subdirectory(standalone)
endif()
