# This CMakeLists file just builds the support library.
# Use the CMakeLists file under opmock2 to build opmock2.

project(opmock_support)

cmake_minimum_required (VERSION 2.8)

add_library(opmock_support STATIC opmock.c)
target_include_directories(opmock_support INTERFACE
						   ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS opmock_support
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
