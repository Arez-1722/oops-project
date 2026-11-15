cmake_minimum_required(VERSION 3.15)
project(ERL_HUB VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(erl_hub ${SOURCES})

# Automated Test Harness
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_suite.cpp")
    add_executable(test_suite 
        src/Cargo.cpp 
        src/Vehicle.cpp 
        src/Depot.cpp 
        tests/test_suite.cpp
    )
endif()
