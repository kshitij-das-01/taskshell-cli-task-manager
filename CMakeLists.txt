cmake_minimum_required(VERSION 4.3)
project(taskshell_cli_task_manager LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core Domain Library
add_executable(taskshell_cli_task_manager main.cpp
        include/task.h
        src/task.cpp
        include/task_manager.h
        src/task_manager.cpp)
