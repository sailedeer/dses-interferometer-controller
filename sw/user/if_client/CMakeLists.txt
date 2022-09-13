cmake_minimum_required(VERSION 3.10)

# set the project name
project(if_client VERSION 0.1)

# pass version number to header file
configure_file(if_client.version if_client.hpp)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# dependencies
find_package(PahoMqttCpp REQUIRED)

# add the executable
add_executable(if_client if_client.cpp)

target_link_libraries(if_client paho-mqttpp3 paho-mqtt3as pthread)


target_include_directories(if_client PUBLIC "${PROJECT_BINARY_DIR}")