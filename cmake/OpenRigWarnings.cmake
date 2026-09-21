add_library(openrig_project_warnings INTERFACE)

if(MSVC)
    target_compile_options(openrig_project_warnings INTERFACE /W4 /permissive-)
    if(OPENRIG_WARNINGS_AS_ERRORS)
        target_compile_options(openrig_project_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(openrig_project_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )
    if(OPENRIG_WARNINGS_AS_ERRORS)
        target_compile_options(openrig_project_warnings INTERFACE -Werror)
    endif()
endif()
