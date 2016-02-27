# Copyright (c) 2016 Andrew Sutton
# All rights reserved

cmake_minimum_required(VERSION 3.0)

project(hashing CXX)

set(CMAKE_CXX_FLAGS "-std=c++1z")

# FIXME: Yuck.
include_directories(~/opt/bin .)

add_library(hashing hashing.cpp)

add_executable(hash_debug hashing.test/debug.cpp)
