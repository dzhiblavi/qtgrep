cmake_minimum_required(VERSION 2.8)

project(test_grepui)

set(CMAKE_CXX_STANDARD 17)

#include_directories(${vector_SOURCE_DIR})

add_executable(test_threadpool
               thread_pool.h
               test_threadpool.cpp)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -D_GLIBCXX_DEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")
endif()

