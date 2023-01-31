cmake_minimum_required(VERSION 3.16)
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${PROJECT_VERSION})

message(STATUS ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(../version.h.in ../../src/version.h)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

set(DEPENDENTS libcurl gsbutils)
include(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED ${DEPENDENTS})

foreach(flag ${PKGS_CFLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endforeach(flag)
foreach(flag ${PKGS_LDFLAGS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
endforeach(flag)

add_library(${PROJECT_NAME} SHARED tlg32.cpp)

target_link_libraries(${PROJECT_NAME} ${CMAKE_EXE_LINKER_FLAGS})
target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
