cmake_minimum_required(VERSION 3.10)
project(esh_project C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)
set(CMAKE_C_STANDARD 99)

set(SOURCES
    src/main.c
    src/esh.c
)

add_executable(esh ${SOURCES})

target_include_directories(esh PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/esh
    DEPENDS esh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)