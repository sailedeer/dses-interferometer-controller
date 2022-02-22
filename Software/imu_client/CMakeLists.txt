cmake_minimum_required(VERSION 3.10)

# set the project name
project(imu_client VERSION 0.1)

# pass version number to header file
configure_file(imu_client.hpp.in imu_client.hpp)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(imu_client imu_client.cpp)

target_include_directories(imu_client PUBLIC "${PROJECT_BINARY_DIR}")