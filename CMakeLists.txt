#
# NOTE: determine required cmake here (use 'pip install cmake')
#
cmake_minimum_required(VERSION 3.21)

#
# NOTE: Include our own cmake scripts, and the CPM package manager
#
include(cmake/CPM.cmake)
include(cmake/print.cmake)
include(cmake/utils.cmake)
extras_mon_get_version()
project(
  extras_mon
  VERSION ${EXTRAS_MON_VERSION}
  LANGUAGES CXX
)

#
# NOTE: Set our source and include directory variables for use through the build
#
set(SOURCES "${PROJECT_SOURCE_DIR}/src")
set(INCLUDES "${PROJECT_SOURCE_DIR}/include")

#
# NOTE: Prevent in source builds (can't build in src/ or in project root)
#
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR OR PROJECT_BINARY_DIR STREQUAL SOURCES)
  print(
    FATAL_ERROR
    "In source builds are not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()
print(STATUS "Building ${PROJECT_NAME}-${PROJECT_VERSION}")

#
# NOTE: bring in standard cmake includes
#
include(CMakeDependentOption)
include(GNUInstallDirs)
include(CheckIPOSupported)

#
# NOTE: Export compile_commands.json file to support clang tooling
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# NOTE: Set C/C++ parameters for all targets in this CMakeLists.txt
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic --std=c++17 -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -fPIC")

#
# NOTE: add additional project options
#
option(EXTRAS_MON_PRODUCTION "Production build" OFF)

#
# MAKE_EXTRAS_MON_LIBRARY_ONLY
#
# Quite often when you include a library from a github project, (via CPM)
# all you are interested in are the shared library files. When you use this
# flag you have to set it on any executables (see below) that you setup
# and/or wish to make optional. Mind you, this option has not been fully
# tested for use with EXTRAS_MON_PRODUCTION flag. So, there might be 
# a hiccup in here that needs to be worked out, (reader beware).
#
# see also MAKE_EXTRAS_MON_LIBRARY_TOOLS
#
# This second flag will allow both the shared libraries and any executatbles
# (but not the unit tests) to be compiled.
#
# Using these flags speeds up the CPM inclusions in other projects significantly.
#

message(STATUS "MAKE_EXTRAS_MON_LIBRARY_ONLY is ${MAKE_EXTRAS_MON_LIBRARY_ONLY}")
if(MAKE_EXTRAS_MON_LIBRARY_ONLY)
  option(BUILD_TESTS "Build tests" OFF)
  option(DEBUG "Debug Build" OFF)
  set(EXTRAS_MON_TESTS OFF)
  message(STATUS "BUILD_TESTS is ${BUILD_TESTS}")
  message(STATUS "DEBUG is ${DEBUG}")
else()
  option(BUILD_TESTS "Build tests" ON)
  option(DEBUG "Debug Build" ON)
  message(STATUS "BUILD_TESTS is ${BUILD_TESTS}")
  message(STATUS "DEBUG is ${DEBUG}")

  #
  # NOTE: Build defaults to production, cmake -DEXTRAS_MON_PRODUCTION=OFF will build debug with tests
  #
  if(NOT EXTRAS_MON_PRODUCTION)
    print(STATUS "Building ${PROJECT_NAME}-${PROJECT_VERSION} [DEBUG]")
    set(CMAKE_BUILD_TYPE Debug)
    set(EXTRAS_MON_TESTS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
  else()
    print(STATUS "Building ${PROJECT_NAME}-${PROJECT_VERSION} [RELEASE]")
    set(EXTRAS_MON_TESTS OFF)
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  endif()

endif()

#
# NOTE: Output build param information
#
print(STATUS "edit your ~/.bashrc and set")
print(STATUS "export LD_LIBRARY_PATH=/usr/local/lib")
print(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
print(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

#
# NOTE: "Include 3rd party libraries, Perry and I maintain an open-source extras C++ library, which
# is used extensively in extras_mon, it comes bundled with spdlog, cpr, and nlohmann json. extras has
# project options that allow us to control how other libraries it includes are built. for example we
# can tell extras to build spdlog as a static library (for faster compile times) by settings
# MAKE_SPDLOG_SHARED OFF" -- Matt Williams, October, 2021
#
CPMAddPackage(
  NAME extras_rsi
  GITHUB_REPOSITORY perriera/extras_rsi
  VERSION 4.21.0
  OPTIONS "MAKE_SPDLOG_SHARED OFF"
  OPTIONS "MAKE_EXTRAS_LIBRARY_ONLY ON"
  OPTIONS "MAKE_EXPARX_ARC_LIBRARY_ONLY ON"
  OPTIONS "MAKE_EXTRAS_ARC_LIBRARY_TOOLS ON"
  OPTIONS "MAKE_EXPARX_RSI_LIBRARY_ONLY ON"
  OPTIONS "MAKE_EXPARX_RSI_LIBRARY_TOOLS ON"
)
if(extras_rsi_ADDED)
  #
  # NOTE:  enable c++11 to avoid compilation errors, and force spdlog into release build
  #
  print(STATUS "Configuring extras_rsi build properties")
  set_target_properties(extras_rsi PROPERTIES CMAKE_BUILD_TYPE Release)
else()
  print(WARNING "extras_rsi was not configured properly")
endif()


#
# HACK: glob src files and header files
#
file(GLOB_RECURSE EXTRAS_MON_HEADERS CONFIGURE_DEPENDS "${INCLUDES}/${PROJECT_NAME}/*.hpp")
file(GLOB_RECURSE EXTRAS_MON_SOURCE CONFIGURE_DEPENDS "${SOURCES}/*.cpp")

#
# NOTE: declare libraries here all source should be placed into a single library, (whether they be
# shared or static)
#
add_library(${PROJECT_NAME} SHARED ${EXTRAS_MON_HEADERS} ${EXTRAS_MON_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs extras_rsi extras_arc extras )

#
# NOTE: declare unit test cases here all source should have adequate test cases whether they be mock
# or unit tests, (or both)
#
if (NOT MAKE_EXTRAS_MON_LIBRARY_ONLY)
  if(EXTRAS_MON_TESTS)
    print(STATUS "Building tests")
    file(GLOB_RECURSE EXTRAS_MON_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
    set(TEST_EXEC "run-unittests-${PROJECT_NAME}")
    add_executable(${TEST_EXEC} ${EXTRAS_MON_TESTS})
    target_include_directories(${TEST_EXEC} PUBLIC ${INCLUDES})
    target_link_libraries(${TEST_EXEC} PRIVATE ${PROJECT_NAME} stdc++fs extras_rsi extras_arc extras )
  endif()
endif()

#
# TODO: declare example client usage application code here, this is where regression tests should
# ultimately be written. as well, this make file uses a GLOB_RECURSE macro which automatically 
# includes ALL headers and sources under include/ and src/. So, to keep two or more main() 
# declarations seperate, we place any sources that have a main() in the instances/ folder
#
if( NOT MAKE_EXTRAS_MON_LIBRARY_ONLY OR MAKE_EXTRAS_MON_LIBRARY_TOOLS)
add_executable(extras_mon_app instances/main.cpp) 
target_include_directories(extras_mon_app PUBLIC ${INCLUDES})
target_link_libraries(extras_mon_app PRIVATE ${PROJECT_NAME} stdc++fs extras_rsi extras_arc extras )
endif()

#
# NOTE: Install all shared libraries
#
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

#
#     #
#     # NOTE: Install all shared libraries & executables, (sample)
#     #
#     install(TARGETS 
#          parcel
#          socketpool_client socketpool_server
#          uploader_client uploader_server 
#          vendor_client vendor_server 
#          downloader_client downloader_server 
#          ${PROJECT_NAME}
#          LIBRARY DESTINATION lib
#          RUNTIME DESTINATION bin)
#     install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
#

#
# NOTE: if the compiler supports iter-procedural optimization, turn it on
#
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  print(STATUS "Building ${PROJECT_NAME} with Interprocedural Optimization")
  set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  print(WARNING "IPO is not supported: ${output}")
endif()

set_target_properties(
  ${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION
                                                                ${${PROJECT_NAME}_VERSION_MAJOR}
)
