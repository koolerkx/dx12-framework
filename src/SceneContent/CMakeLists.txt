include(${CMAKE_SOURCE_DIR}/src/generated/shader/shader-codegen.cmake)

set(SCENE_CONTENT_SOURCES
    scene_registration.h
    scene_registration.cpp

    Scenes/cube_scene/cube_scene.h
    Scenes/cube_scene/cube_scene.cpp

    Scenes/model_scene/model_scene.h
    Scenes/model_scene/model_scene.cpp

    Scenes/test_scene/test_scene.h
    Scenes/test_scene/test_scene.cpp
    Scenes/test_scene/character_mover_component.h
    Scenes/test_scene/character_mover_component.cpp

    Scenes/title_scene/title_scene.h
    Scenes/title_scene/title_scene.cpp

    Scenes/city_scene/city_scene_config.h
    Scenes/city_scene/city_scene_events.h
    Scenes/city_scene/city_scene.h
    Scenes/city_scene/city_scene.cpp
    Scenes/city_scene/object_movement_component.h
    Scenes/city_scene/object_movement_component.cpp
    Scenes/city_scene/player_control_component.h
    Scenes/city_scene/player_control_component.cpp
    Scenes/city_scene/tower_placement_component.h
    Scenes/city_scene/tower_placement_component.cpp
    Scenes/city_scene/enemy_component.h
    Scenes/city_scene/hp_bar_component.h
    Scenes/city_scene/hp_bar_component.cpp
    Scenes/city_scene/turret_component.h
    Scenes/city_scene/turret_component.cpp
    Scenes/city_scene/turret_system.h
    Scenes/city_scene/turret_system.cpp
    Scenes/city_scene/projectile_component.h
    Scenes/city_scene/projectile_component.cpp
    Scenes/city_scene/enemy_spawn_manager_component.h
    Scenes/city_scene/enemy_spawn_manager_component.cpp
    Scenes/city_scene/wave_stage_config.h
    Scenes/city_scene/currency_component.h
    Scenes/city_scene/currency_component.cpp
    Scenes/city_scene/health_component.h
    Scenes/city_scene/base_health_component.h
    Scenes/city_scene/base_health_component.cpp
    Scenes/city_scene/game_match_component.h
    Scenes/city_scene/game_match_component.cpp
    Scenes/city_scene/wave_controller_component.h
    Scenes/city_scene/wave_controller_component.cpp
    Scenes/city_scene/wave_system.h
    Scenes/city_scene/wave_system.cpp
    Scenes/city_scene/pulse_path_component.h
    Scenes/city_scene/pulse_path_component.cpp
    Scenes/city_scene/floating_text_component.h
    Scenes/city_scene/floating_text_effect.h
    Scenes/city_scene/hud_manager_component.h
    Scenes/city_scene/hud_manager_component.cpp
)

function(setup_scene_target target_name game_target)
    add_library(${target_name} STATIC ${SCENE_CONTENT_SOURCES})
    set_msvc_runtime(${target_name})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
    target_link_libraries(${target_name} PUBLIC ${game_target})
    target_link_shader_codegen(${target_name})
endfunction()

setup_scene_target(scene_content game)

if(ENABLE_EDITOR)
    setup_scene_target(scene_content_editor game_editor)
endif()
