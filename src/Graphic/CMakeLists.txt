include(shader)

add_library(graphic STATIC
  graphic.cpp
  graphic.h
  render_service.h
  render_service.cpp
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

  Rendering/post_process_config.h

  Pipeline/pipeline_state_builder.h
  Pipeline/pipeline_state_builder.cpp
  Pipeline/root_signature_builder.h
  Pipeline/root_signature_builder.cpp
  Pipeline/sampler_builder.h
  Pipeline/sampler_builder.cpp
  Pipeline/root_parameter_slots.h
  Pipeline/shader_types.h
  Pipeline/shader_registry.h
  Pipeline/shader_registration.h
  Pipeline/vertex_format_mapping.h
  Pipeline/shader_manager.h
  Pipeline/shader_manager.cpp
  Pipeline/material.h
  Pipeline/material_manager.h
  Pipeline/material_manager.cpp

  Resource/Material/material_descriptor_pool.h
  Resource/Material/material_descriptor_pool.cpp

  Resource/Allocator/free_block_allocator.h
  Resource/Allocator/free_block_allocator.cpp

  Resource/Mesh/mesh_descriptor.h
  Resource/Mesh/mesh_buffer_pool.h
  Resource/Mesh/mesh_buffer_pool.cpp

  Resource/mesh.h
  Resource/mesh.cpp
  Resource/mesh_registry.h
  Resource/render_services.h
  Resource/render_services.cpp
  Resource/Texture/texture.h
  Resource/Texture/texture_manager.h
  Resource/Texture/texture_manager.cpp
  Resource/Font/sprite_font_manager.h
  Resource/Font/sprite_font_manager.cpp

  Resource/texture_service.h
  Resource/texture_service.cpp
  Resource/mesh_service.h
  Resource/mesh_service.cpp
  Resource/font_service.h
  Resource/font_service.cpp

  debug_draw_service.h
  debug_draw_service.cpp

  Frame/per_frame_constant_buffer.h
  Frame/dynamic_upload_buffer.h
  Frame/render_frame_context.h
  Frame/constant_buffers.h
  Frame/resolved_draw_command.h
  Frame/object_data.h
  Frame/object_data_buffer.h
  Frame/object_data_buffer.cpp

  Command/buffer.h
  Command/command_context.h
  Command/command_context.cpp
  Command/render_command_list.h

  Render/render_graph.h
  Render/render_graph.cpp
  Render/render_pass.h
  Render/fullscreen_pass.h
  Render/material_renderer.h
  Render/material_renderer.cpp
  Render/draw_command_resolver.h
  Render/draw_command_resolver.cpp
  Render/resolved_command_grouper.h
  Render/resolved_command_grouper.cpp

  Render/render_texture.h
  Render/render_texture.cpp

  Render/material_pass.h
  Render/material_pass.cpp

  Debug/debug_line_renderer.h
  Debug/debug_line_renderer.cpp

  Render/debug_pass.h
  Render/debug_pass.cpp

  Render/tone_map_pass.h

  Render/depth_view_pass.h

  Render/skybox_pass.h
  Render/skybox_pass.cpp

  Render/blit_pass.h

  Render/shadow_pass.h
  Render/shadow_pass.cpp

  Render/bloom_pass_group.h
  Render/bloom_pass_group.cpp

  Render/pass_group.h
  Render/pass_group.cpp
  Render/shadow_pass_group.h
  Render/shadow_pass_group.cpp
  Render/prepass_group.h
  Render/prepass_group.cpp
  Render/post_process_group.h
  Render/post_process_group.cpp
  Render/preview_group.h
  Render/preview_group.cpp

  Render/depth_normal_pass.h
  Render/depth_normal_pass.cpp

  Render/normal_view_pass.h

  Render/linear_depth_view_pass.h

  Render/ssao_pass_group.h
  Render/ssao_pass_group.cpp

  Render/smaa_pass_group.h
  Render/smaa_pass_group.cpp

  Render/fog_pass.h
  Render/fog_pass.cpp

  Render/outline_pass.h
  Render/outline_pass.cpp

  Render/ui_blur_pass_group.h
  Render/ui_blur_pass_group.cpp

  Render/vignette_pass.h
  Render/vignette_pass.cpp

  Render/chromatic_aberration_pass.h
  Render/chromatic_aberration_pass.cpp

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
  "${CMAKE_SOURCE_DIR}/shaders/soft_particle.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/debug_line.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/debug_line.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/debug_line_instanced.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/fullscreen.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/tonemap.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/skybox.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/skybox.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/depthview.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/blit.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/shadow_depth.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/pbr.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/pbr.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/bloom_downsample.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/bloom_upsample.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/depth_normal.vs.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/depth_normal.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/normal_view.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/linear_depth_view.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/ssao.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/ssao_blur.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/smaa_edge.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/smaa_blend_weight.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/smaa_neighborhood.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/outline.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/vignette.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/fog.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/neon_grid.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/radar_range.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/laser_beam.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/path_pulse.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/chromatic_aberration.ps.hlsl"
  "${CMAKE_SOURCE_DIR}/shaders/ui_glass.ps.hlsl"
)

# manually add shaders files
# target_add_hlsl(graphic "5.0" "vs"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_vs.hlsl"
# )
# target_add_hlsl(graphic "5.0" "Pixel"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_ps.hlsl"
# )

include(fetch)

if(DYNAMIC_CRT)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library" FORCE)
else()
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library")
endif()

set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "" FORCE)

FetchContent_MSVC(DirectXTex https://github.com/microsoft/DirectXTex.git oct2025)
target_link_libraries(graphic PUBLIC DirectXTex)
