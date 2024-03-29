cmake_minimum_required(VERSION 3.20)
project(telebot32 LANGUAGES CXX VERSION 0.11.58)

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")
message(STATUS ${CMAKE_BUILD_TYPE})

# check main project or not
set(MAIN_PROJECT OFF)
set(PREFIX "")
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDebug=1 -DDEBUG=1")
    set(DEBUG_STRING "#define DEBUG 1")
endif()

set(PREFIX /usr/local)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
   set(PREFIX /opt/local)
endif()

set(VERSION ${PROJECT_VERSION})
set(PROJECT_DESCRIPTION "Библиотека телеграм-бота")

set(INCLUDE_INSTALL_DIR ${PREFIX}/include )
set(LIB_INSTALL_DIR ${PREFIX}/lib )
set(SYSCONFIG_INSTALL_DIR "" )

add_subdirectory(src)

if (TEST)
    add_subdirectory(example)
endif()

if (MAIN_PROJECT)
include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
      INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}
      PATH_VARS INCLUDE_INSTALL_DIR 
      SYSCONFIG_INSTALL_DIR
      )
write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
      VERSION ${VERSION}
      COMPATIBILITY SameMajorVersion )  

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_INSTALL_DIR})
install(FILES src/tlg32.h DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
      DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}) 

configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif()