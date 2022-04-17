cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_executable( spaceball_tests
    spaceball_tests.cc
)

target_link_libraries( spaceball_tests
    gtest_main spaceball
)

# needed to counteract the -no_adhoc_codesign thats 
# automatically added by xcode generator on arm64
# see https://gitlab.kitware.com/cmake/cmake/-/issues/21845
if(CMAKE_GENERATOR STREQUAL "Xcode" AND
    "${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
    target_link_options(const_tests PRIVATE "-Xlinker" "-adhoc_codesign")

    # Also... the setting below is supposed to fix this issue, but it doesn't.
    #set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "-o linker-signed")
endif()


include(GoogleTest)
gtest_discover_tests( spaceball_tests )