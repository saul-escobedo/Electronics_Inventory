cmake_minimum_required(VERSION 3.16)

project(Electronics_Inventory VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Try to locate qtpaths (Qt's own path helper)
find_program(QTPATHS_EXECUTABLE NAMES qtpaths qtpaths6 qtpaths-qt6)

if (QTPATHS_EXECUTABLE)
    execute_process(
        COMMAND ${QTPATHS_EXECUTABLE} --install-prefix
        OUTPUT_VARIABLE QT_PREFIX_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Detected Qt prefix: ${QT_PREFIX_PATH}")
    list(APPEND CMAKE_PREFIX_PATH "${QT_PREFIX_PATH}")
endif()

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

add_executable(${PROJECT_NAME} app/src/main.cpp)

# Include the `app` subproject and link its library
add_subdirectory(app)

target_link_libraries(${PROJECT_NAME} PRIVATE 
	electronics_app
	Qt6::Core
	Qt6::Widgets
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

message(STATUS "Configured ${PROJECT_NAME} (minimal CMake)")