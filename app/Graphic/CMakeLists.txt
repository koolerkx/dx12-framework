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

    Resource/mesh.h
    Resource/mesh.cpp
    Resource/Texture/texture.h
    Resource/Texture/texture_manager.h
    Resource/Texture/texture_manager.cpp

    Frame/frame_packet.h
    Frame/per_frame_constant_buffer.h
    Frame/dynamic_upload_buffer.h
    Frame/render_frame_context.h
    Frame/constant_buffers.h

    Command/buffer.h
    Command/render_command_list.h

    Render/render_pass_manager.h
    Render/render_pass_manager.cpp

    Render/ui_pass.h
    Render/ui_pass.cpp
    Render/ui_renderer.h
    Render/ui_renderer.cpp
)

target_link_libraries(app PRIVATE framework)

target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_msvc_runtime(graphic)

target_add_hlsl_auto(graphic "6.5"
    "${CMAKE_SOURCE_DIR}/shaders/basic.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/basic.ps.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/sprite.ps.hlsl"
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
