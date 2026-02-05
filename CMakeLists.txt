cmake_minimum_required(VERSION 3.16)

project(Electronics_Inventory VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} app/src/main.cpp)
# Include the `app` subproject and link its library
add_subdirectory(app)
target_link_libraries(${PROJECT_NAME} PRIVATE electronics_app)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

message(STATUS "Configured ${PROJECT_NAME} (minimal CMake)")