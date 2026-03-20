add_library(framework STATIC
  Core/types.h
  Core/utils.h

  # Handle
  Handle/handle.h

  # GameInput
  Input/input.h
  Input/keyboard.h
  Input/keyboard_handler.h
  Input/mouse.h
  Input/mouse_handler.h
  Input/gamepad.h
  Input/gamepad_handler.h

  # Event System
  Event/event_bus.cpp
  Event/event_bus.hpp
  Event/event.hpp
  Event/event_scope.hpp
  Event/cancellation_token.hpp
  Event/subject_id.hpp
  Event/task.hpp
  Event/task_awaiter.hpp
  Event/task_extensions.hpp
  Event/thread_pool.hpp
  Event/timeout_guard.hpp
  Event/coro_task.hpp
  Event/input_events.h
  Event/input_event_generator.h

  # Font
  Font/bmfont_parser.h
  Font/bmfont_parser.cpp
  Font/text_layout.h
  Font/text_layout.cpp

  # Model
  Model/model_types.h
  Model/model_data.h
  Model/node_hierarchy.h
  Model/model_loader.h

  # Error
  Error/framework_bootstrap_log.h
  Error/framework_bootstrap_log.cpp

  # Utils
  Utils/path_utils.h

  # Logging
  Logging/logger.h
  Logging/logger.cpp
  Logging/logger_config.h
  Logging/sinks.h
  Logging/sinks/console_sink.cpp
  Logging/sinks/debug_sink.cpp
  Logging/sinks/file_sink.cpp

  # Math
  Math/Math.h
  Math/Math.cpp

  # UUID
  UUID/uuid.h

  # ProceduralTexture
  ProceduralTexture/procedural_texture_factory.h

  # Render
  Render/render_handles.h
  Render/render_types.h
  Render/render_settings.h
  Render/render_request.h
  Render/instance_data.h
  Render/camera_data.h
  Render/shadow_config.h
  Render/shader_types.h
  Render/material_descriptor.h
  Render/frame_packet.h
  Render/shader_ids.h
  Render/shader_name_service.h

  # Serialization
  Serialize/serialize_node.h
  Serialize/serialize_node.cpp
  Serialize/serialize_node_impl.h
  Serialize/serialize_document.h
  Serialize/serialize_document.cpp
)

include(fetch)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_MSVC(yaml-cpp https://github.com/jbeder/yaml-cpp.git 0.8.0)
unset(CMAKE_POLICY_VERSION_MINIMUM)

target_link_libraries(framework PUBLIC
  GameInput.lib
  yaml-cpp::yaml-cpp
)

FetchContent_BASE(uuid_v4 https://github.com/crashoz/uuid_v4.git v1.0.0 EXCLUDE_FROM_ALL)

if(DYNAMIC_CRT)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC Runtime Library" FORCE)
else()
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library")
endif()
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE) # GLB/GLIF
# set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_BLEND_IMPORTER ON CACHE BOOL "" FORCE)
# set(ASSIMP_BUILD_USD_IMPORTER ON CACHE BOOL "" FORCE) # USD/USDA/USDC/USDZ

FetchContent_BASE(assimp https://github.com/assimp/assimp.git v6.0.2)
set_target_properties(assimp PROPERTIES MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase") # Zi
# set_target_properties(assimp PROPERTIES MSVC_DEBUG_INFORMATION_FORMAT "Embedded") # Z7

target_link_libraries(framework PUBLIC assimp)


set_msvc_runtime(framework)

target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(framework PUBLIC ${uuid_v4_SOURCE_DIR})
