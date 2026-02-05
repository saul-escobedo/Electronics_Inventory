cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Build implementation as an OBJECT library so internal headers remain private
add_library(electronics_app_impl OBJECT ${APP_SOURCES})
target_include_directories(electronics_app_impl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Public-facing library composed from implementation objects
add_library(electronics_app STATIC $<TARGET_OBJECTS:electronics_app_impl>)
target_include_directories(electronics_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

message(STATUS "Configured app subproject (public headers exposed; src kept private)")
