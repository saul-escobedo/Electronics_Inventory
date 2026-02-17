cmake_minimum_required(VERSION 3.20)

project(ecim_impl)

# Find Qt6 required components
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Source files must be explicitly added because globbing files could
# unintentionally include wrong or temporary CPP files. It also makes
# reviewing new source files easier since adding sources will be
# more intentional.
set(APP_SOURCES
	src/Application.cpp
)

# Build implementation as an OBJECT library
add_library(${PROJECT_NAME} OBJECT ${APP_SOURCES})
# All header files will be localized in the 'include' directory
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
	Qt6::Core
	Qt6::Widgets
)
