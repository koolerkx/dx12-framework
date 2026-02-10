include(shader)

add_library(graphic STATIC
    graphic.cpp
    graphic.h
    d3dx12.h

    Device/gpu_resource.h
    Device/gpu_resource.cpp
    Device/fence_manager.h
    Device/fence_manager.cpp
    Device/device_context.h
    Device/device_context.cpp
    Device/frame_synchronizer.h
    Device/frame_synchronizer.cpp

    Descriptor/descriptor_heap_allocator.h
    Descriptor/descriptor_heap_allocator.cpp
    Descriptor/descriptor_heap_manager.h
    Descriptor/descriptor_heap_manager.cpp

    Presentation/swapchain_manager.h
    Presentation/swapchain_manager.cpp
    Presentation/depth_buffer.h
    Presentation/depth_buffer.cpp
    Presentation/presentation_context.h
    Presentation/presentation_context.cpp

    Rendering/hdr_config.h

    Pipeline/pipeline_state_builder.h
    Pipeline/pipeline_state_builder.cpp
    Pipeline/root_signature_builder.h
    Pipeline/root_signature_builder.cpp
    Pipeline/sampler_builder.h
    Pipeline/sampler_builder.cpp
    Pipeline/root_parameter_slots.h
    Pipeline/shader_types.h
    Pipeline/shader_registry.h
    Pipeline/shader_manager.h
    Pipeline/shader_manager.cpp
    Pipeline/material.h
    Pipeline/material_manager.h
    Pipeline/material_manager.cpp

    Resource/mesh.h
    Resource/mesh.cpp
    Resource/render_services.h
    Resource/render_services.cpp
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
    Frame/render_layer.h
    Frame/draw_command.h

    Command/buffer.h
    Command/command_context.h
    Command/command_context.cpp
    Command/render_command_list.h
    Command/render_command_list.cpp

    Render/render_graph.h
    Render/render_graph.cpp
    Render/render_pass.h
    Render/render_graph_handle.h
    Render/material_renderer.h
    Render/material_renderer.cpp
    Render/draw_command_aggregator.h
    Render/draw_command_aggregator.cpp

    Render/render_texture.h
    Render/render_texture.cpp

    Render/material_pass.h
    Render/material_pass.cpp

    Debug/debug_line_renderer.h
    Debug/debug_line_renderer.cpp

    Render/debug_pass.h
    Render/debug_pass.cpp

    Render/tone_map_pass.h
    Render/tone_map_pass.cpp

    Render/depth_view_pass.h
    Render/depth_view_pass.cpp

    Render/skybox_pass.h
    Render/skybox_pass.cpp

    Render/blit_pass.h
    Render/blit_pass.cpp
)

target_link_libraries(graphic PUBLIC framework)

target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_msvc_runtime(graphic)

target_add_hlsl_auto(graphic "6.6"
    "${CMAKE_SOURCE_DIR}/shaders/basic.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/basic.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite_instanced.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/debug_line.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/debug_line.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/fullscreen.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/tonemap.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/skybox.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/skybox.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/depthview.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/blit.ps.hlsl"
)

# manually add shaders files
# target_add_hlsl(graphic "5.0" "vs"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_vs.hlsl"
# )
# target_add_hlsl(graphic "5.0" "Pixel"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_ps.hlsl"
# )

include(fetch)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library")

FetchContent_MSVC(DirectXTex https://github.com/microsoft/DirectXTex.git oct2025)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(DirectXTex)
target_link_libraries(graphic PUBLIC DirectXTex)
