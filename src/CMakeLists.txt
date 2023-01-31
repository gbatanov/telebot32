cmake_minimum_required(VERSION 3.16)
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${PROJECT_VERSION})

message(STATUS ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(../version.h.in ../../src/version.h)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread -fPIC")

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

SET(DEPENDENTS "libcurl" "gsbutils")
INCLUDE(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED ${DEPENDENTS})

FOREACH(flag ${PKGS_CFLAGS})
    SET(EXTRA_LIB_CFLAGS "${EXTRA_LIB_CFLAGS} ${flag}")
ENDFOREACH(flag)

add_library(${PROJECT_NAME} SHARED tlg32.cpp)

target_link_libraries(${PROJECT_NAME} ${PKGS_LDFLAGS})
target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
