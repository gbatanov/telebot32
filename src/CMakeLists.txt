cmake_minimum_required(VERSION 3.20)
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${PROJECT_VERSION})

message(STATUS ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread ")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

#set(DEPENDENTS "libcurl")
#include(FindPkgConfig)
#pkg_check_modules(PKGS REQUIRED ${DEPENDENTS})

#FOREACH(flag ${PKGS_CFLAGS})
#    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
#ENDFOREACH(flag)
#
#FOREACH(flag ${PKGS_LDFLAGS})
#    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
#ENDFOREACH(flag)

#string(STRIP ${CMAKE_EXE_LINKER_FLAGS} CMAKE_EXE_LINKER_FLAGS)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
set(CURL_DIR "/usr/local/share/vcpkg/packages/curl_x64-osx/share/curl/")
else()
set(CURL_DIR "/usr/local/share/vcpkg/packages/curl_x64-linux/share/curl")
endif()

if(${MAIN_PROJECT})
    configure_file(../version.h.in ../../src/version.h)
    add_library(${PROJECT_NAME} SHARED tlg32.cpp)
    target_link_libraries(${PROJECT_NAME} ${PKGS_LDFLAGS} gsbutils)
    target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
else()
#message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
    add_library(${PROJECT_NAME} STATIC tlg32.cpp)
    target_include_directories(${PROJECT_NAME} PUBLIC ./..)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_EXE_LINKER_FLAGS} gsbutils)
endif()

find_package(CURL CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} CURL::libcurl)
