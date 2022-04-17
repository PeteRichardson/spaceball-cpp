CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

project(demo)

add_executable( demo "main.cc" )

target_link_libraries( demo spaceball )

target_include_directories(demo PUBLIC "../../spaceball")
