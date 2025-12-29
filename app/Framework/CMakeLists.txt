add_library(framework INTERFACE
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

  # Event
  Event/event_system.h
  Event/event_types.h

  # Custom Event
  Event/input_events.h
  Event/input_event_generator.h
)

target_link_libraries(framework INTERFACE
  GameInput.lib
)

# set_msvc_runtime(framework)

target_include_directories(framework INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
