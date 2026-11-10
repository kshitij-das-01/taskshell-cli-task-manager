cmake_minimum_required(VERSION 4.3)
project(taskshell_cli_task_manager LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_CLI_APP "Build native CLI application for local testing" OFF)
option(BUILD_TESTS "Build unit tests (native only)" OFF)

# Core Domain Library
add_library(task_manager STATIC
    src/task.cpp
    src/task_manager.cpp
)

target_include_directories(task_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Native CLI (local testing only)
if (BUILD_CLI_APP)
    add_executable(taskshell_cli
        src/cli_app.cpp
    )
    target_link_libraries(taskshell_cli PRIVATE task_manager)
endif()

# Native CLI Tests
if (BUILD_TESTS AND NOT EMSCRIPTEN)
    enable_testing()

    add_executable(task_manager_tests
        test/task_manager_tests.cpp
    )

    target_link_libraries(task_manager_tests PRIVATE
        task_manager
    )

    add_test(
        NAME TaskManagerTests
        COMMAND task_manager_tests
    )

endif()

# WASM Build (only active under emcmake)
if (EMSCRIPTEN)
    add_executable(taskshell_wasm
        src/wasm_bindings.cpp
    )

    target_include_directories(taskshell_wasm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(taskshell_wasm PRIVATE task_manager)

    set_target_properties(taskshell_wasm PROPERTIES
        OUTPUT_NAME "task_manager"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web
    )

    target_link_options(taskshell_wasm PRIVATE
        "--bind"
        "--no-entry"
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sEXPORT_NAME=createTaskManagerModule"
        "-sENVIRONMENT=web"
        "-sWASM_BIGINT=1"
        "-sALLOW_MEMORY_GROWTH=1"
    )
endif()
