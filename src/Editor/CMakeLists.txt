include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.5-docking
)

FetchContent_Declare(
  imguizmo
  GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
  GIT_TAG a15acd87a3f3241a29ea1363ceafc680dca3a96b
)

FetchContent_MakeAvailable(imgui imguizmo)

# ImGui static library (core + Win32/DX12 backends + ImGuizmo)
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
  ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${imguizmo_SOURCE_DIR}
)

target_link_libraries(imgui PRIVATE d3d12.lib dxgi.lib)

set_msvc_runtime(imgui)

# Suppress warnings for third-party code
if(MSVC)
  target_compile_options(imgui PRIVATE /W0)
endif()

# Editor static library
add_library(editor STATIC
  editor_layer.h
  editor_layer.cpp
  editor_ui_imgui.cpp
)

target_include_directories(editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Framework)

target_link_libraries(editor PUBLIC imgui)
target_compile_definitions(editor PRIVATE ENABLE_EDITOR=1)
target_link_libraries(editor PRIVATE game_editor graphic)

set_msvc_runtime(editor)
