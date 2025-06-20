cmake_minimum_required(VERSION 3.10)
project(OS_LAB_3)

set(CMAKE_CXX_STANDARD 17)

add_executable(OS_LAB_3 
    src/main.cpp 
    src/marker.cpp
)

add_executable(tests 
    test/test.cpp 
    src/marker.cpp
    include/catch_amalgamated.cpp
)

target_include_directories(tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(OS_LAB_3 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)