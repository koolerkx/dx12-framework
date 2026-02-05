set(APP_SOURCES
    main.cpp
    Application/Application.h
    Application/Application.cpp
    Application/timer_updater.h
    Application/timer_updater.cpp
)

add_subdirectory(Framework)
add_subdirectory(Graphic)
add_subdirectory(Game)

function(setup_app_target target_name)
    set_msvc_runtime(${target_name})
    if(WIN32)
        target_compile_definitions(${target_name} PRIVATE _WIN32_WINNT=0x0A00)
    endif()
    target_link_libraries(${target_name} PRIVATE
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
        uuid.lib
        framework
        graphic
        game
    )
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Framework)
endfunction()

add_executable(app WIN32 ${APP_SOURCES})
setup_app_target(app)

if(ENABLE_EDITOR)
    add_subdirectory(Editor)

    add_executable(app_editor WIN32 ${APP_SOURCES})
    setup_app_target(app_editor)
    target_compile_definitions(app_editor PRIVATE ENABLE_EDITOR=1)
    target_link_libraries(app_editor PRIVATE editor)
endif()
