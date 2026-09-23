project(OpenRigStandalone VERSION 0.1.0)

set(OPENRIG_STANDALONE_DEFINES NO_IGRAPHICS)
if(WIN32 AND NOT OPENRIG_IPLUG2_ENABLE_ASIO)
    list(APPEND OPENRIG_STANDALONE_DEFINES OPENRIG_ASIO_DISABLED)
endif()

iplug_add_plugin(${PROJECT_NAME}
    SOURCES
        OpenRigStandalone.cpp
        OpenRigStandalone.h
        AsioDisabled.cpp
        resources/resource.h
    FORMATS
        APP
    UI
        NONE
    DEFINES
        ${OPENRIG_STANDALONE_DEFINES}
    LINK
        OpenRig::IPlugAdapter
)
