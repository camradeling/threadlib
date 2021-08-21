cmake_minimum_required(VERSION 3.6)
project(ProgramThread)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_NAME ProgramThread)

set(Sources
        programthread.cpp
        )

add_library(${PROJECT_NAME} SHARED ${Sources} ${version_file})