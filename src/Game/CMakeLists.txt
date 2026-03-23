add_library(game STATIC
    game.h
    game.cpp

    game_context.h

    scene_key.h
    scene_events.h
    scene_manager.h
    scene_manager.cpp

    scene.h
    scene.cpp
    scene_defaults.h

    # Asset files moved to Framework/Asset/

    game_object.h
    game_object.cpp

    Component/component.h
    Component/component.cpp
    Component/renderer_component.h
    Component/renderer_component.cpp
    Component/transform_component.h
    Component/transform_component.cpp
    Component/camera_component.h
    Component/camera_component.cpp
    Component/behavior_component.h
    Component/billboard_type.h
    Component/sprite_sheet_animator.h
    Component/sprite_sheet_animator.cpp
    Component/pivot_type.h
    Component/pivot_type.cpp
    Component/sprite_sheet_helper.h
    Component/sprite_sheet_helper.cpp
    Component/model_component.h
    Component/Renderer/mesh_renderer.h
    Component/Renderer/sprite_renderer.h
    Component/Renderer/sprite_renderer.cpp
    Component/Renderer/text_renderer.h
    Component/Renderer/text_renderer.cpp
    Component/Renderer/ui_sprite_renderer.h
    Component/Renderer/ui_sprite_renderer.cpp
    Component/Renderer/ui_glass_renderer.h
    Component/Renderer/ui_glass_renderer.cpp
    Component/Renderer/ui_text_renderer.h
    Component/Renderer/ui_text_renderer.cpp
    Component/Renderer/particle_emitter.h
    Component/Renderer/particle_emitter.cpp
    Component/Renderer/instanced_model_renderer.h
    Component/Renderer/instanced_model_renderer.cpp
    Component/Renderer/instanced_mesh_renderer.h
    Component/Renderer/instanced_mesh_renderer.cpp
    Component/Collider/collider_component.h
    Component/Collider/box_collider_component.h
    Component/Collider/box_collider_component.cpp
    Component/Collider/sphere_collider_component.h
    Component/Collider/sphere_collider_component.cpp
    Component/nav_grid_component.h
    Component/nav_grid_component.cpp
    Component/player_spawn_component.h
    Component/enemy_spawn_component.h

    Serialization/scene_serializer.h
    Serialization/scene_serializer.cpp

    SceneSetting/active_camera_setting.h
    SceneSetting/active_camera_setting.cpp
    SceneSetting/active_ui_camera_setting.h
    SceneSetting/active_ui_camera_setting.cpp
    SceneSetting/background_setting.h
    SceneSetting/background_setting.cpp
    SceneSetting/shadow_setting.h
    SceneSetting/scene_transition_overlay.h
    SceneSetting/scene_transition_overlay.cpp

    Scripts/free_camera_controller.h
    Scripts/free_camera_controller.cpp

    Debug/debug_drawer.h
    Debug/debug_drawer.cpp

    Scenes/empty_scene.h

    System/system.h
    System/render_system.h
    System/render_system.cpp

    Map/map_data.h
    Map/ground_ray_caster.h
    Map/map_data.cpp
    Map/map_loader.h
    Map/map_loader.cpp
    Map/a_star.h
    Map/a_star.cpp
    Map/nav_grid.h
    Map/nav_grid.cpp
    Map/pathfinder.h
    Map/pathfinder.cpp

    Shaders/game_shaders.h
)

set_msvc_runtime(game)

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

target_link_libraries(game PUBLIC framework)
