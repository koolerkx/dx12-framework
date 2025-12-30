add_library(game STATIC
    game.h
    game.cpp

    game_context.h

    scene.h
    scene.cpp

    Asset/asset_handle.h
    Asset/asset_manager.h
    Asset/asset_manager.cpp

    game_object.h
    game_object.cpp

    Component/component.h
    Component/component.cpp
    Component/transform_component.h
    Component/transform_component.cpp
    Component/camera_component.h
    Component/camera_component.cpp
    Component/free_camera_controller.h
    Component/free_camera_controller.cpp

    Scenes/test_scene/test_scene.h
    Scenes/test_scene/test_scene.cpp
    Scenes/test_scene/character_mover_component.h
    Scenes/test_scene/character_mover_component.cpp
)

set_msvc_runtime(game)

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

target_link_libraries(game PUBLIC graphic)
