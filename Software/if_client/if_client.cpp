#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <mqtt/async_client.h>
// #include <if/lsm6ds032.h>
// #include <if/tbh67h303.h>

using namespace std;

// TODO: write ini config file which this program will read from
const string SERVER_ADDRESS("ssl://localhost:1883");
const string CLIENT_ID("if_test");
const string SUB_TOPIC_AZ("cmd/az/" + CLIENT_ID);
const string SUB_TOPIC_EL("cmd/el/" + CLIENT_ID);
const string SUB_TOPIC_MODE("cmd/mode/" + CLIENT_ID);
const string PUB_TOPIC_IMU("sensors/imu/" + CLIENT_ID);
const string PUB_TOPIC_ENC("sensors/enc/" + CLIENT_ID);

const int QOS = 1;

// TODO: write driver for imu
void publish_imu_data(mqtt::async_client_ptr client /*, if::LSM6DS032_ptr imu */) {

}

void pub_enc_data() {

}

void process_el() {

}

void process_az() {

}

void process_cmd(string &cmd) {

}

int main(int argc, char **argv) {
    // use a smart pointer since we'll have several publisher threads
    auto client = make_shared<mqtt::async_client>(SERVER_ADDRESS, CLIENT_ID);
    
    auto conn_opts = mqtt::connect_options_builder()
                    .clean_session(false)
                    .automatic_reconnect(chrono::seconds(2), chrono::seconds(30))
                    .finalize();

    /*
     *
     * Basic Setup:
     *      unique thread for each pub topic
     *      unique callback for each sub topic
     *          (i.e. if we receive a command to increase elevation, raise the "elevation" callback)
     *      main thread consumes messages from underlying queue, and invokes a handler callback for each message sub-type
     *
     */ 



    // bad boy spinloop
    while(true) {
        
    }
    return EXIT_SUCCESS;
}