cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the project name
project(TicTacToe LANGUAGES CXX)

# add the executable
add_executable(tictactoe main.cpp)
target_compile_features(tictactoe PRIVATE cxx_std_20)
set_target_properties(tictactoe PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(tictactoe PRIVATE -O3 -Wall -Wextra -Wpedantic)

add_executable(test test.cpp)
target_compile_features(test PRIVATE cxx_std_20)
set_target_properties(test PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(test PRIVATE -O3 -Wall -Wextra -Wpedantic)
