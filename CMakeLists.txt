cmake_minimum_required(VERSION 3.16)
project(telebot32 LANGUAGES CXX VERSION 0.7.31)

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")
message(STATUS ${CMAKE_BUILD_TYPE})

# check main project or not
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
    set(USEUTILS "#define USE_UTILS 1")
else()
    set(USEUTILS "")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDebug=1 -DDEBUG=1  -g")
    set(DEBUG_STRING "#define DEBUG 1")
endif()

if(${MAIN_PROJECT})
  configure_file(version.h.in ../src/version.h)
else()
  configure_file(../version.h.in ../../telebot32/src/version.h)
endif()

add_subdirectory(src)

if(${MAIN_PROJECT})
if (TEST)
 add_subdirectory(example)
 endif()
endif()
