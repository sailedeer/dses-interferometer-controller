cmake_minimum_required(VERSION 3.19)

add_library(if-common)

# target_include_directories(if-common
#     INTERFACE

# )

add_executable(if-client
    src/if-client.cpp
)

# add_executable(if-server
#     src/if-server.cpp
# )

# target_include_directories(if-client

# )

# dependencies
find_package(PahoMqttCpp REQUIRED)

# add the executable
add_executable(if_client
    if_client.cpp
)

target_link_libraries(if_client
    paho-mqttpp3
    paho-mqtt3as
    pthread
)

target_include_directories(if_client
    PUBLIC
        "${PROJECT_BINARY_DIR}"
)
