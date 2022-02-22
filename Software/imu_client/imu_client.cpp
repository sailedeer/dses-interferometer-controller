#include <iostream>

#include <imu_client.hpp>

int main(int argc, char **argv) {
    std::cout << "IMU MQTT Client" << std::endl << std::flush;
    std::cout << "Version " << imu_client_VERSION_MAJOR << "." << imu_client_VERSION_MINOR << std::endl;
    return 0;
}