include(shader)

add_library(graphic STATIC
    graphic.cpp
    graphic.h
    d3dx12.h

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

target_link_libraries(app PRIVATE framework)

target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(graphic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework)
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