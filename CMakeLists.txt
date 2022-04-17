CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

option(CMAKE_VERBOSE_MAKEFILE "Show commands while building" ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS true)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)

project(spaceball)

add_subdirectory( spaceball )
add_subdirectory( spaceball-examples )


