add_library(framework INTERFACE
  Core/types.h
  Core/utils.h

  # DirectXTK Input
  Input/input.h
  Input/keyboard.h
  Input/keyboard_handler.h
  Input/mouse.h
  Input/mouse_handler.h
  Input/gamepad.h
  Input/gamepad_handler.h
)

target_link_libraries(framework INTERFACE
  GameInput.lib
)

# set_msvc_runtime(framework)

target_include_directories(framework INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
