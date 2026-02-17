cmake_minimum_required(VERSION 3.20)

# Project Name & executable name
project(ecim VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include the app implementation subdirectory
add_subdirectory(app)

# Create executable from app implementation
add_executable(${PROJECT_NAME} app/src/main.cpp)

# Link libraries and include directories
target_link_libraries(${PROJECT_NAME} PRIVATE ecim_impl)

# Generate compile commands because some IDEs use clangd language servers
# for auto-complete and code suggestions
if (PROJECT_IS_TOP_LEVEL AND UNIX)
	execute_process(
    	COMMAND ${CMAKE_COMMAND} -E create_symlink
    	${CMAKE_BINARY_DIR}/compile_commands.json
    	${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
	)
endif()

message(STATUS "Configured ${PROJECT_NAME} (v${PROJECT_VERSION})")
