cmake_minimum_required(VERSION 2.8)
project(Kosmonaut)
set(Kosmonaut_LIBRARY_NAME kosmonaut)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(Kosmonaut_DIR "${CMAKE_SOURCE_DIR}/src")

find_package(Threads REQUIRED)

list(APPEND Kosmonaut_LIBRARIES
  ${CMAKE_THREAD_LIBS_INIT}
  ${CMAKE_SOURCE_DIR}/deps/czmq/src/libczmq.la
  )

list(APPEND Kosmonaut_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/deps/czmq/include
  )

list(APPEND Kosmonaut_SOURCES
  src/kosmonaut.c
  )

add_library(${Kosmonaut_LIBRARY_NAME} ${Kosmonaut_SOURCES})
include_directories(${Kosmonaut_LIBRARY_NAME} ${Kosmonaut_INCLUDE_DIRS})
target_link_libraries(${Kosmonaut_LIBRARY_NAME} ${Kosmonaut_LIBRARIES})