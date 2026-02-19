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
	src/electronic_components.cpp
	src/electronics_manager.cpp
)

# Build implementation as an OBJECT library
add_library(${PROJECT_NAME} OBJECT ${APP_SOURCES})
# All header files will be localized in the 'include' directory
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
	Qt6::Core
	Qt6::Widgets
)


# testing section, be careful
option(ENABLE_TESTING "Enable building tests" ON)

if(ENABLE_TESTING)
	add_executable(test_components 
		src/electronic_components.cpp
		src/electronics_manager.cpp
		tests/test_components.cpp 
	)

	target_include_directories(test_components PRIVATE include)
	set_target_properties(test_components PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

	# Enable CTest and register the test executable so `ctest`/`cmake --build --target check` can run it
	include(CTest)
	enable_testing()
	add_test(NAME test_components COMMAND test_components)

	# Convenience target to run ctest with output on failure
	add_custom_target(check
		COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
		DEPENDS test_components
	)
endif()
