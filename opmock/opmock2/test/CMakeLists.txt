cmake_minimum_required (VERSION 2.8)

include( CTest )

find_path(SNOWBIRD_SOURCE_DIR "build.py" "../../../..")

# Define macro for adding unit test exectuables.
function(create_unit_test UNIT_TEST_NAME TEST_SRC MOCK_SRC TEST_LIBS)
  include_directories(${SNOWBIRD_SOURCE_DIR}/third-party/cpputest-3.7.1/include)
  include_directories(${SNOWBIRD_SOURCE_DIR}/third-party/opmock2.1/opmock2/support)
  include_directories(${SNOWBIRD_SOURCE_DIR}/test_support)
  include_directories(${PROJECT_BINARY_DIR}/mocks)
  #add assert_to_stderr.cpp for windows builds so we can get console output on smoke
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(TEST_SRC ${TEST_SRC} # Append
      ${SNOWBIRD_SOURCE_DIR}/sys/debug/win32/assert_to_stderr.cpp
      )
  endif()

  string(REGEX REPLACE "([^;]+)" "${MOCK_DIR}/\\1" full_mock_src "${MOCK_SRC}")
  set_source_files_properties(${full_mock_src} PROPERTIES GENERATED TRUE)
  set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${UNIT_TEST_NAME} PARENT_SCOPE)
  add_executable(${UNIT_TEST_NAME} ${TEST_SRC} ${full_mock_src})
  target_link_libraries(${UNIT_TEST_NAME}
    CppUTest CppUTestExt
	opmock_support
    ${TEST_LIBS})
  set_target_properties(${UNIT_TEST_NAME} PROPERTIES LINKER_LANGUAGE C)
  add_test(${UNIT_TEST_NAME} ${UNIT_TEST_NAME})

  foreach(f ${MOCK_SRC})
    get_filename_component(TARGET ${f} NAME_WE)
    get_filename_component(DIR ${f} DIRECTORY)
    if ("${DIR}" STREQUAL "")
      add_dependencies(${UNIT_TEST_NAME} "${TARGET}")
    else()
      string(REPLACE "/" "_" DIR "${DIR}")
      add_dependencies(${UNIT_TEST_NAME} "${DIR}_${TARGET}")
    endif()
  endforeach(f)
endfunction()

add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)

#define DEBUG for debug builds
#the normal convention is to define NDEBUG for release builds
#but we already have code that expects DEBUG, so we'll use it
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC) # Windows
    include_directories(${SNOWBIRD_SOURCE_DIR}/sys/msvc/compiler)
    include_directories(${SNOWBIRD_SOURCE_DIR}/sys/debug/win32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    #this next line can be used to prevent automatic regeneration of visual studio project files
    #alternatively, this option can be set when running cmake with
    #cmake -DCMAKE_SUPPRESS_REGENERATION:BOOL=ON
    #set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "require manual run of cmake" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FC /W3 /EHsc /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC /W3 /EHsc /WX")

elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU) # True for both Linux and Cortex-A53 builds.

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=gnu99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++0x")

    #coverage option
    option(CODE_COVERAGE "CODE_COVERAGE" OFF)
    SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov -coverage -fprofile-arcs -ftest-coverage")
    if (${CODE_COVERAGE} STREQUAL ON)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
      set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
    endif ()

    include_directories(${SNOWBIRD_SOURCE_DIR}/sys/gcc/compiler)
else()
    message("your compiler is not currently handled")
endif ()

# Add common includes
include_directories(${PROJECT_SOURCE_DIR}/test)
include_directories(${SNOWBIRD_SOURCE_DIR}/third-party/cpputest-3.7.1/include)

set(OPMOCK_FLAGS "-cpp"
                 "-std=c++11"
                 "-q")

# Set up auto mock generation.

# Add commands for generating mocks for all headers under "test".  These
# commands will only be executed if a unit test adds a mock "stub" file to
# its list of source files.  Consequently, only mocks for interfaces that
# are being mocked are generated.
file(GLOB_RECURSE mockable_headers RELATIVE ${PROJECT_SOURCE_DIR}/test *.h)

foreach(header ${mockable_headers})
  get_filename_component(dir ${header} DIRECTORY)
  get_filename_component(name ${header} NAME)
  get_filename_component(base ${header} NAME_WE)
  string(REPLACE "/" "_" ndir "${dir}")
  if ("${dir}" STREQUAL "")
	set(mock_file mocks/${base}_stub.cpp)
	set(mock_target ${base}_stub)
  else()
    set(mock_file mocks/${dir}/${base}_stub.cpp)
    set(mock_target ${ndir}_${base}_stub)
  endif()
  add_custom_command(
    OUTPUT ${mock_file}
	# Before sending the file to opmock, we comment out any "#pragma once"
	# directive because opmock warns if the 'main file' contains it.
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/mocks/${dir}
    COMMAND opmock2 ${OPMOCK_INCLUDES} ${OPMOCK_FLAGS}
	  -i ${PROJECT_SOURCE_DIR}/test/${header}
	  -o ${PROJECT_BINARY_DIR}/mocks/${dir}
    MAIN_DEPENDENCY ${header}
    DEPENDS opmock2
    COMMENT "Mocking ${name}")
  add_custom_target(${mock_target} DEPENDS ${mock_file})
  add_dependencies(${mock_target} opmock2)
  set_target_properties (${mock_target} PROPERTIES FOLDER Mocks)
endforeach(header)

set(MOCK_DIR "${PROJECT_BINARY_DIR}/mocks")


# Now that everything is set up, add our subdirectories.
set(TESTS OFF CACHE BOOL "Turn off CppUTest's own unit tests" FORCE)
add_subdirectory (${SNOWBIRD_SOURCE_DIR}/third-party/cpputest-3.7.1 ${PROJECT_BINARY_DIR}/third-party/cpputest-3.7.1)
add_subdirectory (${SNOWBIRD_SOURCE_DIR}/third-party/opmock2.1 ${PROJECT_BINARY_DIR}/third-party/opmock2.1)


# Unit test
set(TEST_SRC
    ignore_and_return_unittest.cpp
)

set(MOCK_SRC
    funcs_stub.cpp
)

set(TEST_LIBS
)

create_unit_test(ignore_and_return "${TEST_SRC}" "${MOCK_SRC}" "${TEST_LIBS}")


# Unit test
set(TEST_SRC
    ignore_other_calls_unittest.cpp
)

set(MOCK_SRC
    funcs_stub.cpp
    funcs_other_stub.cpp
)

set(TEST_LIBS
)

create_unit_test(ignore_other_calls "${TEST_SRC}" "${MOCK_SRC}" "${TEST_LIBS}")

# Run the unit tests after building them.
add_custom_target( all_tests ALL
                   DEPENDS ${UNIT_TEST_TARGETS}
)

add_custom_command(TARGET all_tests
                   COMMENT "Run tests"
                   POST_BUILD COMMAND ctest ARGS --output-on-failure
)
