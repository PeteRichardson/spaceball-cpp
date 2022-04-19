CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

project(spaceball)

add_library (spaceball STATIC
    "spaceball.cc" "spaceball.h" "serial.cc"
)

# add_subdirectory(tests)

target_compile_features(spaceball PUBLIC cxx_std_20)
