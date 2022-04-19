CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

add_executable( serial_connect "main.cc" "serial.cc")

target_compile_features(serial_connect PUBLIC cxx_std_20)
