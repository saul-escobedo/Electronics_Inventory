cmake_minimum_required(VERSION 3.20)

project(ecim_impl)

# Find Qt6 required components
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(Python3 COMPONENTS Interpreter)

set(ECIM_PYTHON_EXECUTABLE "python3")
if(Python3_Interpreter_FOUND)
	set(ECIM_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
endif()

# Source files must be explicitly added because globbing files could
# unintentionally include wrong or temporary CPP files. It also makes
# reviewing new source files easier since adding sources will be
# more intentional.
set(APP_SOURCES
	src/Application.cpp
	src/DistributorImportSchema.cpp
	src/PythonScriptRunner.cpp
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
target_compile_definitions(${PROJECT_NAME} PUBLIC
	ECIM_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
	ECIM_PYTHON_EXECUTABLE="${ECIM_PYTHON_EXECUTABLE}"
)


# testing section, be careful
option(ENABLE_TESTING "Enable building tests" ON)

if(ENABLE_TESTING)
	include(FetchContent)

	FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/refs/heads/main.zip
		# Ensure extracted files get current timestamps to avoid CMP0135 warning
		DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	)
	# Prevent overriding the parent project's compiler/linker settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	# Create test executable; linking to the OBJECT library includes its objects
	# and transitively propagates Qt6 interface dependencies (PUBLIC linkage)
	add_executable(my_tests
		tests/test_components.cpp
		tests/test_distributor_import_schema.cpp
		tests/test_python_script_runner.cpp
	)

	target_include_directories(my_tests PRIVATE include)
	set_target_properties(my_tests PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

	target_link_libraries(my_tests PRIVATE ${PROJECT_NAME} gtest gtest_main)

	# Register the test executable with CTest (enable_testing is called in the root CMakeLists)
	add_test(NAME my_tests COMMAND my_tests)

	# Convenience target to run ctest with output on failure
	add_custom_target(check
		COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
		DEPENDS my_tests
	)
endif()
