include(shader)

add_library(graphic STATIC
    graphic.cpp
    graphic.h
    d3dx12.h

    Device/gpu_resource.h
    Device/gpu_resource.cpp
    Device/fence_manager.h
    Device/fence_manager.cpp

    Descriptor/descriptor_heap_allocator.h
    Descriptor/descriptor_heap_allocator.cpp
    Descriptor/descriptor_heap_manager.h
    Descriptor/descriptor_heap_manager.cpp

    Presentation/swapchain_manager.h
    Presentation/swapchain_manager.cpp
    Presentation/depth_buffer.h
    Presentation/depth_buffer.cpp

    Pipeline/pipeline_state_builder.h
    Pipeline/pipeline_state_builder.cpp
    Pipeline/root_signature_builder.h
    Pipeline/root_signature_builder.cpp
    Pipeline/sampler_builder.h
    Pipeline/sampler_builder.cpp
    Pipeline/root_parameter_slots.h
    Pipeline/material.h
    Pipeline/material_manager.h
    Pipeline/material_manager.cpp
    Pipeline/shader_config.h

    Resource/mesh.h
    Resource/mesh.cpp
    Resource/Texture/texture.h
    Resource/Texture/texture_manager.h
    Resource/Texture/texture_manager.cpp
    Resource/Font/sprite_font_manager.h
    Resource/Font/sprite_font_manager.cpp

    Frame/frame_packet.h
    Frame/per_frame_constant_buffer.h
    Frame/dynamic_upload_buffer.h
    Frame/render_frame_context.h
    Frame/constant_buffers.h
    Frame/camera_data.h

    Command/buffer.h
    Command/render_command_list.h
    Command/render_command_list.cpp

    Render/render_pass_manager.h
    Render/render_pass_manager.cpp
    Render/render_pass.h
    Render/material_renderer.h
    Render/material_renderer.cpp

    Render/ui_pass.h
    Render/ui_pass.cpp

    Render/opaque_pass.h
    Render/opaque_pass.cpp

    Render/transparent_pass.h
    Render/transparent_pass.cpp

    Debug/debug_line_renderer.h
    Debug/debug_line_renderer.cpp

    Render/debug_pass.h
    Render/debug_pass.cpp
)

target_link_libraries(app PRIVATE framework)

target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_msvc_runtime(graphic)

target_add_hlsl_auto(graphic "6.6"
    "${CMAKE_SOURCE_DIR}/shaders/basic.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/basic.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced_ui.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced_ui.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced_world.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced_world.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced_world_transparent.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced_world_transparent.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/debug_line.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/debug_line.ps.hlsl"
)

# manually add shaders files
# target_add_hlsl(graphic "5.0" "vs"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_vs.hlsl"
# )
# target_add_hlsl(graphic "5.0" "Pixel"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_ps.hlsl"
# )

# Require CMake 3.11
include(FetchContent)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library")

FetchContent_Declare(
    DirectXTex
    GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
    GIT_TAG oct2025
)
FetchContent_MakeAvailable(DirectXTex)
target_link_libraries(graphic PUBLIC DirectXTex)
