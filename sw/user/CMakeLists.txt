cmake_minimum_required(VERSION 3.19)

# set the project name
project(dses-interferometer)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(comm)
add_subdirectory(drivers)
