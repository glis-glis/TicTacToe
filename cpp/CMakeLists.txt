cmake_minimum_required(VERSION 3.10)

# set the project name
project(TicTacToe)

# add the executable
add_executable(test test.cpp)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS -std=c++2a)
