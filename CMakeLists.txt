cmake_minimum_required(VERSION 3.6)
project(ProgramThread)

set(CMAKE_CXX_STANDARD 17)
set(BIN_DIR ${CMAKE_BINARY_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})
set(PROJECT_NAME ProgramThread)
#submodules////////////////////////////////////////////////////////////////////////////////////////////////////////////////
find_package(Git QUIET)   
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "git submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()
#//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
include_directories(messagequeue)
set(Sources
        programthread.cpp
        )

add_library(${PROJECT_NAME} SHARED ${Sources})
add_library(${PROJECT_NAME}_static STATIC ${Sources})