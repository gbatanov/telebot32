cmake_minimum_required(VERSION 3.20)
project(test LANGUAGES CXX VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/local/lib ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/opt/local/include -pthread")
else()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -I/usr/local/include -pthread")
endif()

SET(EXECUTABLE_OUTPUT_PATH bin)

configure_file(../botname.h.in ../../example/botname.h)

ADD_EXECUTABLE(${PROJECT_NAME} telega32.cpp)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} telebot32 gsbutils)
target_include_directories(${PROJECT_NAME} PUBLIC tlg32 /usr/local/include /opt/local/include)

if(${MAIN_PROJECT})
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/local/bin)
  message(STATUS "Bot name ${BOT_NAME}")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/.token DESTINATION /usr/local/etc/telebot32/.token${BOT_NAME})
endif()
