cmake_minimum_required(VERSION 2.8)
project(Astronaut)
set(Astronaut_LIBRARY_NAME astronaut)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(Astronaut_DIR "${CMAKE_SOURCE_DIR}/src")

find_package(Threads REQUIRED)

list(APPEND Astronaut_LIBRARIES
  ${CMAKE_THREAD_LIBS_INIT}
  ${CMAKE_SOURCE_DIR}/deps/czmq/src/libczmq.la
  )

list(APPEND Astronaut_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/deps/czmq/include
  )

list(APPEND Astronaut_SOURCES
  src/astronaut.c
  )

add_library(${Astronaut_LIBRARY_NAME} ${Astronaut_SOURCES})
include_directories(${Astronaut_LIBRARY_NAME} ${Astronaut_INCLUDE_DIRS})
target_link_libraries(${Astronaut_LIBRARY_NAME} ${Astronaut_LIBRARIES})