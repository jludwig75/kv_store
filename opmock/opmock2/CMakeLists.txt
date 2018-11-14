cmake_minimum_required (VERSION 2.8)
project (opmock2)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(opmock2 opmock2.cpp)

llvm_map_components_to_libnames(llvm_libs all)

# Remove LTO because it wasn't built in Windows and we don't need it.
list(REMOVE_ITEM llvm_libs LTO)

# Link against LLVM and Clang libraries
target_link_libraries(opmock2
					  ${llvm_libs}
					  clangToolingCore
					  clangAST
					  clangBasic
					  clangFrontend
					  clangParse
					  clangLex
					  clangSema
					  clangSerialization
					  clangDriver
					  clangBasic
					  clangAST
					  clangEdit
					  clangAnalysis)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC) # Windows
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    #this next line can be used to prevent automatic regeneration of visual studio project files
    #alternatively, this option can be set when running cmake with
    #cmake -DCMAKE_SUPPRESS_REGENERATION:BOOL=ON
    #set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "require manual run of cmake" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FC /W3 /EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC /W3 /EHsc")

elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU) # True for both Linux and Cortex-A53 builds.

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -static")

else()
    message("your compiler is not currently handled")
endif ()

add_subdirectory (test)
