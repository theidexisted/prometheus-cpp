cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)

project(prometheus-cpp)
enable_testing()
include(GNUInstallDirs)
add_definitions(-fPIC)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)

if(NOT DEFINED CMAKE_CXX_STANDARD AND UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
endif()

find_package(GoogleBenchmark)
find_package(Protobuf REQUIRED)
find_package(Telegraf)


find_path(FlatBuffersHeader flatbuffers/flatbuffers.h)
find_library(FlatBuffersLibrary NAMES libflatbuffers.a)
message(STATUS "flatbuffer header:" ${FlatBuffersHeader})
message(STATUS "flatbuffer library:" ${FlatBuffersLibrary})

# suppress warnings

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

# civetweb

set(CIVETWEB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/civetweb/include)

set(CIVETWEB_VERSION_MAJOR 1)
set(CIVETWEB_VERSION_MINOR 9)
set(CIVETWEB_VERSION_PATCH 0)
set(CIVETWEB_VERSION_STRING ${CIVETWEB_VERSION_MAJOR}.${CIVETWEB_VERSION_MINOR}.${CIVETWEB_VERSION_PATCH})


add_library(civetweb STATIC
  3rdparty/civetweb/include/CivetServer.h
  3rdparty/civetweb/include/civetweb.h
  3rdparty/civetweb/src/CivetServer.cpp
  3rdparty/civetweb/src/civetweb.c
  3rdparty/civetweb/src/handle_form.inl
  3rdparty/civetweb/src/md5.inl
)

set_target_properties(civetweb PROPERTIES VERSION ${CIVETWEB_VERSION_STRING}
	SOVERSION ${CIVETWEB_VERSION_MAJOR})


target_compile_definitions(civetweb PRIVATE
  USE_IPV6
  NDEBUG
  NO_CGI
  NO_CACHING
  NO_SSL
  NO_FILES
)

target_include_directories(civetweb PUBLIC
  ${CIVETWEB_INCLUDE_DIR}
)

# google mock

add_library(gmock_main STATIC EXCLUDE_FROM_ALL
  3rdparty/googletest/googletest/src/gtest-all.cc
  3rdparty/googletest/googlemock/src/gmock-all.cc
  3rdparty/googletest/googlemock/src/gmock_main.cc
)

target_include_directories(gmock_main
  PUBLIC
    3rdparty/googletest/googletest/include
    3rdparty/googletest/googlemock/include
  PRIVATE
    3rdparty/googletest/googletest
    3rdparty/googletest/googlemock
)

# prometheus-cpp

add_subdirectory(lib)
add_subdirectory(tests)
