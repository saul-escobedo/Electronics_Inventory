cmake_minimum_required(VERSION 3.16)

project(Electronics_Inventory VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Qt6 required components
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Include the app subproject
add_subdirectory(app)

# Create executable from app implementation
add_executable(${PROJECT_NAME} app/src/main.cpp)

# Link libraries and include directories
target_link_libraries(${PROJECT_NAME} PRIVATE electronics_app Qt6::Core Qt6::Widgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

message(STATUS "Configured ${PROJECT_NAME} (v${PROJECT_VERSION})")