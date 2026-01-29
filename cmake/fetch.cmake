# Wrapper for FetchContent
# usage: 
# include(fetch)
# FetchContent_BASE(name repo tag)
# FetchContent_MSVC(name repo tag)

include(FetchContent)

function(FetchContent_BASE name repo tag)
  FetchContent_Declare(
    ${name}
    GIT_REPOSITORY ${repo}
    GIT_TAG ${tag}
  )

  FetchContent_MakeAvailable(${name})
endfunction()

function(FetchContent_MSVC name repo tag)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" PARENT_SCOPE)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  FetchContent_BASE(
    ${name}
    ${repo}
    ${tag}
  )

  unset(CMAKE_MSVC_RUNTIME_LIBRARY PARENT_SCOPE)
endfunction()