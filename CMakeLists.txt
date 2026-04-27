cmake_minimum_required(VERSION 3.16)

project(Electronics_Inventory_User_Interface VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# Find sqlite3
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Generate embedded SQL header
set(DBSQL_SCHEMA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/app/sql/Schema.sql")
set(DBSQL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/app/sql/Accessors.sql;${CMAKE_CURRENT_SOURCE_DIR}/app/sql/Transaction.sql")
set(DBSQL_OUTPUT_HPP "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSQLCode.hpp")
include(${CMAKE_CURRENT_SOURCE_DIR}/app/cmake/EmbedSQL.cmake)

# Add binary directory to include path for generated header
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(app/include)
include_directories(${SQLITE3_INCLUDE_DIRS})

set(PROJECT_SOURCES
        UI/Electronics_Inventory_User_Interface/main.cpp
        UI/Electronics_Inventory_User_Interface/mainwindow.cpp
        UI/Electronics_Inventory_User_Interface/mainwindow.h
        UI/Electronics_Inventory_User_Interface/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Electronics_Inventory_User_Interface
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        UI/Electronics_Inventory_User_Interface/add_item_dialog.h UI/Electronics_Inventory_User_Interface/add_item_dialog.cpp UI/Electronics_Inventory_User_Interface/add_item_dialog.ui
        UI/Electronics_Inventory_User_Interface/view_item_dialog.h UI/Electronics_Inventory_User_Interface/view_item_dialog.cpp UI/Electronics_Inventory_User_Interface/view_item_dialog.ui
        UI/Electronics_Inventory_User_Interface/database.h UI/Electronics_Inventory_User_Interface/database.cpp
        UI/Electronics_Inventory_User_Interface/edit_item_dialog.h UI/Electronics_Inventory_User_Interface/edit_item_dialog.cpp UI/Electronics_Inventory_User_Interface/edit_item_dialog.ui
        UI/Electronics_Inventory_User_Interface/settings.h UI/Electronics_Inventory_User_Interface/settings.cpp UI/Electronics_Inventory_User_Interface/settings.ui
        # Backend sources
        app/src/database/SQLiteDatabase.cpp
        app/src/database/MassQueryConfig.cpp
        app/src/electronics_manager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Electronics_Inventory_User_Interface APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Electronics_Inventory_User_Interface SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Electronics_Inventory_User_Interface
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Electronics_Inventory_User_Interface PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                    Qt${QT_VERSION_MAJOR}::Sql
                    sqlite3)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Electronics_Inventory_User_Interface)
endif()
set_target_properties(Electronics_Inventory_User_Interface PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Electronics_Inventory_User_Interface
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Electronics_Inventory_User_Interface)
endif()
