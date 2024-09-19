cmake_minimum_required(VERSION 3.13)

# root includes
set(ROOT_INCLUDES ${PROJECT_SOURCE_DIR}/include)

# Data files
file(GLOB SRC_ETC_FILE {$PROJECT_SOURCE_DIR}/etc/*)
file(GLOB ETC_FILES RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/etc/*)

# Data directory
file(GLOB SRC_ETC_DIR ${PROJECT_SOURCE_DIR}/etc)

# Tests project
set(PROJECT_NAME combinations_test)
project(${PROJECT_NAME})

# Inlcude directories
include_directories(${ROOT_INCLUDES})

# Source files
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Unit tests
add_executable(runUnitTests ${SRC_FILES})
target_compile_options(runUnitTests PRIVATE ${COMPILE_OPTS} -O3
    -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-function)
target_link_options(runUnitTests PRIVATE ${LINK_OPTS})

# Standard linking to gtest stuff
target_link_libraries(runUnitTests gtest gtest_main)

# Extra linking for the project
target_link_libraries(runUnitTests xml_lib)
target_link_libraries(runUnitTests combinations_lib)

# Copy data files for tests
add_custom_command(
    OUTPUT ${ETC_FILES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_ETC_DIR} "${CMAKE_CURRENT_BINARY_DIR}/../etc"
    DEPENDS ${SRC_ETC_FILES}
    COMMENT "Copying test data")

add_custom_target(
    etc DEPENDS ${ETC_FILES})

add_dependencies(runUnitTests etc)
