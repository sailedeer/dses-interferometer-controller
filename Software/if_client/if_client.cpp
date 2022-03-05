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

// TODO: write config file which this program
const string SERVER_ADDRESS("ssl://localhost:14514");
const string CLIENT_ID("if_test");

const string SUB_TOPIC_AZ("cmd/az/" + CLIENT_ID);
const string SUB_TOPIC_EL("cmd/el/" + CLIENT_ID);
const string SUB_TOPIC_MODE("cmd/mode" + CLIENT_ID);
const string SUB_TOPIC_ROLLCALL("cmd/rollcall");
const string PUB_TOPIC_IMU("sensors/imu/" + CLIENT_ID);
const string PUB_TOPIC_ENC("sensors/enc/" + CLIENT_ID);
const string PUB_TOPIC_ROLLCALL("rollcall");

const int QOS = 1;

const int SENS_RAND_MAX = 1024;

volatile bool asleep = false;

volatile bool die = false;

volatile bool track = false;

mutex asleep_mtx;
mutex die_mtx;
mutex track_mtx;

condition_variable asleep_cv;

// TODO: write driver for imu
void imu_publisher(mqtt::async_client_ptr client /*, if::LSM6DS032_ptr imu */) {
    int random_int;
    bool local_die;
    
    srand(time(0));
    while (true) {
        {
            unique_lock<mutex> lk(asleep_mtx);
            asleep_cv.wait(lk, [] { 
                    return !asleep;
                }
            );
        }
        auto then = chrono::steady_clock::now() + chrono::seconds(5);
        // TODO: collect elevation/azimuthal data from IMU,
        // for now just publish a random number
        random_int = rand() % SENS_RAND_MAX;
        client->publish(PUB_TOPIC_IMU, &random_int, sizeof(int))->wait();
        {
            lock_guard<mutex> lk(die_mtx);
            local_die = die;
        }
        if (die) {
            break;
        } else {
            this_thread::sleep_until(then);
        }
    }
}

void enc_publisher(mqtt::async_client_ptr client) {
    int random_int;
    bool local_die;
    
    srand(time(0));
    while (true) {
        {
            unique_lock<mutex> lk(asleep_mtx);
            asleep_cv.wait(lk, [] { 
                    return !asleep;
                }
            );
        }
        auto then = chrono::steady_clock::now() + chrono::seconds(5);
        // TODO: collect data from encoder,
        // for now just publish a random number
        random_int = rand() % SENS_RAND_MAX;
        client->publish(PUB_TOPIC_ENC, &random_int, sizeof(int))->wait();
        {
            lock_guard<mutex> lk(die_mtx);
            local_die = die;
        }
        if (die) {
            break;
        } else {
            this_thread::sleep_until(then);
        }
    }
}

void command_processor(mqtt::async_client_ptr client) {
    mqtt::const_message_ptr msg;
    string msg_topic;
    bool local_die = false;
    while (true) {
        msg = client->consume_message();
        if (!msg) {
            continue;
        }

        msg_topic = msg->get_topic();
        if (msg_topic == SUB_TOPIC_AZ) {
            cout << "Standin for processing azimuth command...\n";
        } else if (msg_topic == SUB_TOPIC_EL) {
            cout << "Standin for processing elevation command...\n";
        } else if (msg_topic == SUB_TOPIC_MODE) {
            string cmd = msg->get_payload_str();
            if (cmd == "sleep") {
                lock_guard<mutex> lk(asleep_mtx);
                asleep = true;
            } else if (cmd == "wake") {
                {
                    lock_guard<mutex> lk(asleep_mtx);
                    asleep = false;
                }

                // bang pots and pans
                asleep_cv.notify_all();
            } else if (cmd == "exit") {
                lock_guard<mutex> lk(die_mtx);
                die = true;
            } else if (cmd == "track") {
                lock_guard<mutex> lk(track_mtx);
                // begin tracking current position
                // TODO: maybe include lat and long in this
                // in case we want to do some kind 
                track = true;
            } else if (cmd == "hold") {
                // hold current position
                lock_guard<mutex> lk(track_mtx);
                track = false;
            } else {
                cerr << "Unknown mode: " << cmd << '\n';
            }
        } else if (msg_topic == SUB_TOPIC_ROLLCALL &&
                    msg->get_payload_str() == "broadcast") {
            client->publish(PUB_TOPIC_ROLLCALL, CLIENT_ID)->wait();
        } else {
            cerr << "Unrecognized topic: " << msg->get_topic() << '\n';
            cerr << "Discarding packet.\n";
        }

        {
            lock_guard<mutex> lk(die_mtx);
            local_die = die;
        }
        if (die) {
            break;
        }
    }
}

int main(int argc, char **argv) {
    // use a smart pointer since we'll have several threads
    // since this is an async client, all operations are thread-safe
    auto client = make_shared<mqtt::async_client>(SERVER_ADDRESS, CLIENT_ID);
    
    auto conn_opts = mqtt::connect_options_builder()
                    .clean_session(false)
                    .automatic_reconnect(chrono::seconds(2), chrono::seconds(30))
                    .finalize();

    // collections for topics + their corresponding qualities of service
    auto topics = mqtt::string_collection::create({SUB_TOPIC_AZ, SUB_TOPIC_EL, SUB_TOPIC_MODE, SUB_TOPIC_ROLLCALL});
    const vector<int> QOS{2, 2, 2, 2};

    try {
        // start consuming in case we're re-hydrating
        client->start_consuming();
        auto rsp = client->connect(conn_opts)->get_connect_response();

        if (!rsp.is_session_present()) {
            client->subscribe(topics, QOS);
        }

        // publish the client ID onto the rollcall topic so the webserver knows we're alive
        client->publish(PUB_TOPIC_ROLLCALL, CLIENT_ID)->wait();

        // start threads
        thread imu_publisher_thread(imu_publisher, client);
        thread enc_publisher_thread(enc_publisher, client);
        thread command_processor_thread(command_processor, client);

        // command processor thread will signal other threads to die eventually
        imu_publisher_thread.join();
        enc_publisher_thread.join();
        command_processor_thread.join();
    } catch (const mqtt::exception &ex) {
        cerr << ex.what() << endl;
        return EXIT_FAILURE;
    }
    cout << "Exiting..." << endl;
    return EXIT_SUCCESS;
}