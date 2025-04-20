cmake_minimum_required(VERSION 3.6)
project(ProgramThread)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(PROJECT_NAME ProgramThread)
include_directories(messagequeue)
set(Sources
        programthread.cpp
        )

add_library(${PROJECT_NAME} SHARED ${Sources})
add_library(${PROJECT_NAME}_static STATIC ${Sources})
