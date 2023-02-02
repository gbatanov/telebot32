cmake_minimum_required(VERSION 3.16)
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${PROJECT_VERSION})

message(STATUS ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

set(DEPENDENTS "libcurl")
if(${MAIN_PROJECT})
 set(DEPENDENTS "${DEPENDS} gsbutils")
endif()
include(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED ${DEPENDENTS})

FOREACH(flag ${PKGS_CFLAGS})
    SET(EXTRA_LIB_CFLAGS "${EXTRA_LIB_CFLAGS} ${flag}")
ENDFOREACH(flag)

if(${MAIN_PROJECT})
    configure_file(../version.h.in ../../src/version.h)
    add_library(${PROJECT_NAME} SHARED tlg32.cpp)
    target_link_libraries(${PROJECT_NAME} ${PKGS_LDFLAGS})
    target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
else()
#message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
    add_library(${PROJECT_NAME} STATIC tlg32.cpp)
    target_include_directories(${PROJECT_NAME} PUBLIC ./..)
endif()

