cmake_minimum_required(VERSION 3.30)

project(traceroute)

# Main target
add_executable(
    ${PROJECT_NAME}
    src/exceptionService/exceptionservice.h
    src/exceptionService/exceptionservice.cpp
    src/tracerouteService/tracerouteservice.h
    src/tracerouteService/tracerouteservice.cpp
    src/threadService/threadservice.h
    src/threadService/threadservice.cpp
    src/main.cpp
)

if (MSVC)
    target_compile_options(
        # flags for additional logging
        ${PROJECT_NAME} PRIVATE
        /W4
    )
else()
    target_compile_options(
        # flags for additional logging
        ${PROJECT_NAME} PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()
