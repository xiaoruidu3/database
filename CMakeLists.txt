# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Define the project name and version
project(MyProject VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(part01 part01.cpp)

add_executable(part02 part02.cpp)

add_executable(part03 part03.cpp)

