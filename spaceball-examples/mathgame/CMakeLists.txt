cmake_minimum_required(VERSION 3.18)

project(threaded)

add_executable(mathgame main.cc)

target_link_libraries(mathgame spaceball)
target_include_directories(mathgame PUBLIC "../../spaceball")