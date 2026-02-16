cmake_minimum_required(VERSION 3.20)


# Project Name & executable name
project(ecim)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



# Find and locate the developer's installation of Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)



# PROJECT_SOURCES contains all active source files that will be compiled into
# the executable
set(PROJECT_SOURCES
	src/main.cpp
	src/ProjectSetupTest.cpp
)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})



# Project's include directories where all the header files reside (cpp files
# and header files are located in different directories)
target_include_directories(${PROJECT_NAME} PRIVATE
	include
)


# Links all required libraries into the executable
target_link_libraries(${PROJECT_NAME}
	Qt6::Core
	Qt6::Widgets
)



# Generate compile commands because some IDEs uses clangd language servers
# for auto-complete and code suggestions
if (PROJECT_IS_TOP_LEVEL AND UNIX)
	execute_process(
    	COMMAND ${CMAKE_COMMAND} -E create_symlink
    	${CMAKE_BINARY_DIR}/compile_commands.json
    	${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
	)
endif()
