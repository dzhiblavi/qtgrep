cmake_minimum_required(VERSION 2.8)
project(test_grepui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH /usr/local/Cellar/qt/5.13.2)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
include_directories(${test_grepui_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -D_GLIBCXX_DEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")
endif()

add_executable(test_threadpool
               thread_pool.h
               thread_pool.cpp
               task.h
               task.cpp
               test_threadpool.cpp
               grep_task.cpp
               grep_task.h
               gtest/gtest-all.cc
               gtest/gtest.h
               gtest/gtest_main.cc
               fault_injection/fault_injection.cpp
               fault_injection/fault_injection.hpp)

target_link_libraries(test_threadpool -lpthread Qt5::Widgets)


