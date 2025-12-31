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

  # Event
  Event/event_system.h
  Event/event_types.h

  # Custom Event
  Event/input_events.h
  Event/input_event_generator.h

  # Font
  Font/bmfont_parser.h
  Font/bmfont_parser.cpp
  Font/text_layout.h
  Font/text_layout.cpp
)

target_link_libraries(framework PUBLIC
  GameInput.lib
)

set_msvc_runtime(framework)

target_include_directories(framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
