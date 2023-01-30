cmake_minimum_required(VERSION 3.16)
project(telebot32 LANGUAGES CXX VERSION 0.8.36)

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")
message(STATUS ${CMAKE_BUILD_TYPE})

# check main project or not
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PREFIX /usr/local)
    set(MAIN_PROJECT ON)
else()
    set(PREFIX "")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    set(SUFF "")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDebug=1 -DDEBUG=1")
    set(DEBUG_STRING "#define DEBUG 1")
    set(SUFF "d")
endif()

add_subdirectory(gsb_utils)
add_subdirectory(src)

if(${MAIN_PROJECT})
  if (TEST)
    add_subdirectory(example)
  endif()
endif()
