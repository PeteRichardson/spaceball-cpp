CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

FIND_LIBRARY(IOKIT_LIBRARY IOKit)
FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)

add_executable( SerialPortSample "SerialPortSample.cc")
target_link_libraries(SerialPortSample ${IOKIT_LIBRARY} ${FOUNDATION_LIBRARY})

target_compile_features(SerialPortSample PUBLIC cxx_std_20)
