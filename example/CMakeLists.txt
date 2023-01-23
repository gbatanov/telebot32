cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES CXX VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread")

SET(EXECUTABLE_OUTPUT_PATH bin)

configure_file(../botname.h.in ../../example/botname.h)

ADD_EXECUTABLE(${PROJECT_NAME} telega32.cpp)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} telebot32${SUFF})
target_include_directories(${PROJECT_NAME} PUBLIC tlg32)

if(${MAIN_PROJECT})
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/local/bin)
  message(STATUS "Bot name ${BOT_NAME}")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/.token DESTINATION /usr/local/etc/telebot32/.token${BOT_NAME})
endif()
