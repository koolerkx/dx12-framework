# Wrapper for FetchContent
# usage:
# include(fetch)
# FetchContent_BASE(name repo tag [EXCLUDE_FROM_ALL])
# FetchContent_MSVC(name repo tag [EXCLUDE_FROM_ALL])

include(FetchContent)

function(FetchContent_BASE name repo tag)
  set(extra_args ${ARGN})
  list(FIND extra_args "EXCLUDE_FROM_ALL" has_exclude)

  if(has_exclude GREATER_EQUAL 0)
    FetchContent_Declare(
      ${name}
      GIT_REPOSITORY ${repo}
      GIT_TAG ${tag}
      EXCLUDE_FROM_ALL
    )
  else()
    FetchContent_Declare(
      ${name}
      GIT_REPOSITORY ${repo}
      GIT_TAG ${tag}
    )
  endif()

  FetchContent_MakeAvailable(${name})
endfunction()

function(FetchContent_MSVC name repo tag)
  if(DYNAMIC_CRT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" PARENT_SCOPE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" PARENT_SCOPE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()

  FetchContent_BASE(
    ${name}
    ${repo}
    ${tag}
  )

  unset(CMAKE_MSVC_RUNTIME_LIBRARY PARENT_SCOPE)
endfunction()