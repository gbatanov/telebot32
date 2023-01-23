cmake_minimum_required(VERSION 3.16)
project(${PROJECT_NAME} LANGUAGES CXX VERSION ${PROJECT_VERSION})

set(PROJECT_NAME_INITIAL ${PROJECT_NAME})
set(PROJECT_NAME ${PROJECT_NAME}${SUFF})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${MAIN_PROJECT})
  configure_file(../version.h.in ../../src/version.h)
else()
  configure_file(../../version.h.in ../../../${PROJECT_NAME_INITIAL}/src/version.h)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread")

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

SET(DEPENDENTS "libcurl")
INCLUDE(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED ${DEPENDENTS})

FOREACH(flag ${PKGS_CFLAGS})
    SET(EXTRA_LIB_CFLAGS "${EXTRA_LIB_CFLAGS} ${flag}")
ENDFOREACH(flag)

add_library(${PROJECT_NAME} STATIC tlg32.cpp)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PKGS_LDFLAGS} ${LIB_CURL})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/lib)
install(FILES tlg32.h DESTINATION /usr/local/include/${PROJECT_NAME_INITIAL})

