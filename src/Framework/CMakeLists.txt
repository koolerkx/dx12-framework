include(fetch)
FetchContent_BASE(uuid_v4 https://github.com/crashoz/uuid_v4.git v1.0.0 EXCLUDE_FROM_ALL)

add_library(framework STATIC
  Core/types.h
  Core/utils.h

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

  # Serialization
  Serialize/serialize_node.h
  Serialize/serialize_node.cpp
  Serialize/serialize_node_impl.h
  Serialize/serialize_document.h
  Serialize/serialize_document.cpp
)

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

set_msvc_runtime(framework)

target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(framework PUBLIC ${uuid_v4_SOURCE_DIR})
