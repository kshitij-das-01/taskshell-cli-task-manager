cmake_minimum_required(VERSION 4.3)
project(taskshell_cli_task_manager LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_CLI_APP "Build native CLI application for local testing" OFF)
option(BUILD_TESTS "Build unit tests (native only)" OFF)

# Core Domain Library
add_library(task_manger STATIC
    src/task.cpp
    src/task_manager.cpp
)

target_include_directories(task_manger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Native CLI (local testing only)
if (BUILD_CLI_APP)
    add_executable(taskshell_cli
        src/cli_app.cpp
    )
    target_link_libraries(taskshell_cli PRIVATE task_manger)
endif()

#add_executable(taskshell_cli_task_manager
#        include/task.h
#        src/task.cpp
#        include/task_manager.h
#        src/task_manager.cpp
#        src/cli_app.cpp)
