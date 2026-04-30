cmake_minimum_required(VERSION 3.20)

project(ecim_impl)

set(PYBIND11_FINDPYTHON NEW)

# ----- Dependencies -----

include(FetchContent)

# Find Qt6 required components
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Fetch nlohmann/json for settings management
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    pybind11
    URL https://github.com/pybind/pybind11/archive/refs/tags/v3.0.4.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(pybind11)

# Find SQLite3 database engine
find_package(SQLite3)
if(NOT TARGET SQLite3::SQLite3) # CMake < 4.3
    add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
endif()

# ----- EndDependencies -----


# Source files must be explicitly added because globbing files could
# unintentionally include wrong or temporary CPP files. It also makes
# reviewing new source files easier since adding sources will be
# more intentional.
set(APP_SOURCES
	src/electronics_manager.cpp

    src/database/MassQueryConfig.cpp
    src/database/SQLiteDatabase.cpp

	src/settings/Settings.cpp

	# ----- UI Sources -----

	include/ui/MainWindow.hpp
	include/ui/AddItemDialog.hpp
	include/ui/EditItemDialog.hpp
	include/ui/ViewItemDialog.hpp
	include/ui/Settings.hpp

	src/ui/Database.cpp
	src/ui/MainWindow.cpp
	src/ui/AddItemDialog.cpp
	src/ui/EditItemDialog.cpp
	src/ui/ViewItemDialog.cpp
	src/ui/Settings.cpp

	layout/MainWindow.ui
	layout/AddItemDialog.ui
	layout/EditItemDialog.ui
	layout/ViewItemDialog.ui
	layout/Settings.ui
)

# Build implementation as an OBJECT library
add_library(${PROJECT_NAME} OBJECT ${APP_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    # Allow CMake to automatically generate headers files from *.ui layout files
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/layout"

    # Allow CMake to generate Qt signal implementations
    AUTOMOC ON
)

# All header files will be localized in the 'include' directory
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC
	Qt6::Core
	Qt6::Widgets

	nlohmann_json::nlohmann_json

    SQLite3::SQLite3

    pybind11::embed
)

# Embed SQL code into a header file
set(DBSQL_SCHEMA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/sql/Schema.sql")
set(DBSQL_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/sql/Accessors.sql"
    "${CMAKE_CURRENT_SOURCE_DIR}/sql/Transaction.sql"
)
include(cmake/EmbedSQL.cmake)

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
    add_executable(ecim_tests
        tests/test_components.cpp
        tests/test_sqlitedb.cpp
        tests/test_settings.cpp
    )

    target_include_directories(ecim_tests PRIVATE include)
    set_target_properties(ecim_tests PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

    target_link_libraries(ecim_tests PRIVATE ${PROJECT_NAME} gtest gtest_main)

    # Register the test executable with CTest (enable_testing is called in the root CMakeLists)
    add_test(NAME ecim_tests COMMAND ecim_tests)

	# Convenience target to run ctest with output on failure
	add_custom_target(check
		COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
		DEPENDS ecim_tests
	)
endif()
