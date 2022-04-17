CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

project(spaceball)

add_subdirectory( spaceball )
add_subdirectory( spaceball-examples )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set (CMAKE_EXPORT_COMPILE_COMMANDS true)