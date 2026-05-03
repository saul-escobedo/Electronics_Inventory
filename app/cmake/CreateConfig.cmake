cmake_minimum_required(VERSION 3.20)

find_package(Git QUIET)

set(CONFIG_OUTPUT_HPP_DIR "${CMAKE_CURRENT_BINARY_DIR}/config")
set(CONFIG_OUTPUT_HPP "${CONFIG_OUTPUT_HPP_DIR}/Config.hpp")

set(BUILD_BRANCH "unknown")
set(BUILD_HASH "unknown")

function(find_build_metadata)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} "rev-parse" "--is-inside-work-tree"
            OUTPUT_QUIET
            ERROR_QUIET
            RESULT_VARIABLE RET_VAL
        )

        if(NOT RET_VAL EQUAL 0)
            return()
        endif()
    else()
        return()
    endif()

    execute_process(
        COMMAND ${GIT_EXECUTABLE} "branch" "--show-current"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE BUILD_BRANCH
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} "rev-parse" "--short" "HEAD"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE BUILD_HASH
    )

    set(BUILD_BRANCH ${BUILD_BRANCH} PARENT_SCOPE)
    set(BUILD_HASH ${BUILD_HASH} PARENT_SCOPE)
endfunction()

find_build_metadata()

set(HEADER_CONTENT "")
string(APPEND HEADER_CONTENT "#define ECIM_VERSION \"${CMAKE_PROJECT_VERSION}\"\n")
string(APPEND HEADER_CONTENT "#define ECIM_BRANCH \"${BUILD_BRANCH}\"\n")
string(APPEND HEADER_CONTENT "#define ECIM_BUILD \"${BUILD_HASH}\"\n")

file(WRITE "${CONFIG_OUTPUT_HPP}" "${HEADER_CONTENT}")
message(STATUS "Generated config header file: ${CONFIG_OUTPUT_HPP}")

target_include_directories(${PROJECT_NAME} PUBLIC ${CONFIG_OUTPUT_HPP_DIR})
