add_library(game STATIC
    game.h
    game.cpp

    game_context.h

    scene.h
    scene.cpp

    Asset/asset_handle.h
    Asset/asset_manager.h
    Asset/asset_manager.cpp
    Asset/text_mesh_handle.h

    game_object.h
    game_object.cpp

    Component/component.h
    Component/component.cpp
    Component/transform_component.h
    Component/transform_component.cpp
    Component/camera_component.h
    Component/camera_component.cpp
    Component/behavior_component.h
    Component/billboard_helper.h
    Component/billboard_type.h
    Component/render_settings.h
    Component/sprite_sheet_animator.h
    Component/sprite_sheet_animator.cpp
    Component/pivot_type.h
    Component/pivot_type.cpp
    Component/sprite_sheet_helper.h
    Component/sprite_sheet_helper.cpp
    Component/Renderer/mesh_renderer.h
    Component/Renderer/sprite_renderer.h
    Component/Renderer/sprite_renderer.cpp
    Component/Renderer/text_renderer.h
    Component/Renderer/text_renderer.cpp
    Component/Renderer/ui_sprite_renderer.h
    Component/Renderer/ui_sprite_renderer.cpp
    Component/Renderer/ui_text_renderer.h
    Component/Renderer/ui_text_renderer.cpp

    SceneSetting/active_camera_setting.h
    SceneSetting/active_camera_setting.cpp
    SceneSetting/active_ui_camera_setting.h
    SceneSetting/active_ui_camera_setting.cpp

    Scripts/free_camera_controller.h
    Scripts/free_camera_controller.cpp

    Debug/debug_drawer.h
    Debug/debug_drawer.cpp

    Scenes/test_scene/test_scene.h
    Scenes/test_scene/test_scene.cpp
    Scenes/test_scene/character_mover_component.h
    Scenes/test_scene/character_mover_component.cpp
)

set_msvc_runtime(game)

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

target_link_libraries(game PUBLIC graphic)
