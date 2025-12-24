include(shader)

add_library(graphic STATIC
    WICTextureLoader12.h
    WICTextureLoader12.cpp
    graphic.cpp
    graphic.h
    d3dx12.h
    types.h
    ../utils.h

    gpu_resource.h
    gpu_resource.cpp
    descriptor_heap_allocator.h
    descriptor_heap_allocator.cpp
    descriptor_heap_manager.h
    descriptor_heap_manager.cpp

    swapchain_manager.h
    swapchain_manager.cpp
    depth_buffer.h
    depth_buffer.cpp
    fence_manager.h
    fence_manager.cpp

    pipeline_state_builder.h
    pipeline_state_builder.cpp
    root_signature_builder.h
    root_signature_builder.cpp
    sampler_builder.h
    sampler_builder.cpp

    mesh.h
    mesh.cpp

    Texture/texture_manager.h
    Texture/texture_manager.cpp
)

target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_msvc_runtime(graphic)

target_add_hlsl_auto(graphic "6.5"
    "${CMAKE_SOURCE_DIR}/shaders/basic.vs.hlsl"
    "${CMAKE_SOURCE_DIR}/shaders/basic.ps.hlsl"
)

# manually add shaders files
# target_add_hlsl(graphic "5.0" "vs"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_vs.hlsl"
# )
# target_add_hlsl(graphic "5.0" "Pixel"
#     "${CMAKE_SOURCE_DIR}/shaders/basic_ps.hlsl"
# )

