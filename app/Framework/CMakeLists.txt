add_library(framework INTERFACE
  Core/types.h
  Core/utils.h
)

# set_msvc_runtime(framework)

target_include_directories(framework INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
