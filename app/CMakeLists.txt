cmake_minimum_required(VERSION 3.20)

project(ecim_impl)

# Find Qt6 required components
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Find SQLite3 database engine
find_package(SQLite3)
if(NOT TARGET SQLite3::SQLite3) # CMake < 4.3
    add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
endif()

# Source files must be explicitly added because globbing files could
# unintentionally include wrong or temporary CPP files. It also makes
# reviewing new source files easier since adding sources will be
# more intentional.
set(APP_SOURCES
    src/Application.cpp
    src/electronics_manager.cpp

    src/database/SQLiteDatabase.cpp
)

# Build implementation as an OBJECT library
add_library(${PROJECT_NAME} OBJECT ${APP_SOURCES})

# All header files will be localized in the 'include' directory
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt6::Core
    Qt6::Widgets

    SQLite3::SQLite3
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
    add_executable(ecim_tests
        tests/test_components.cpp
        tests/test_sqlitedb.cpp
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
