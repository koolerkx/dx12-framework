add_library(game STATIC
    game.h
    game.cpp

    scene.h
    scene.cpp
)

set_msvc_runtime(game)

target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}../)

target_link_libraries(game PUBLIC graphic)
