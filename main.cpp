//
// Created by sage7 on 3/18/25.
//
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

#include "radar.h"

using namespace std;

///
/// @return 1 on failure, or don't return if everything is going well
int main(){
    // string pipe_name = "radarpipe";
    // mkfifo(pipe_name.c_str(), 0666);
    // cout << "waiting for messages...";
    // int fd = open(pipe_name.c_str(), O_RDONLY);
    // if(fd == -1){
    //     std::cerr << "Pipe error!";
    //     return 1;
    // }

    char buffer_raw[128];
    string buffer(buffer_raw);
    radarData data;
    cmatch match;
    cout << "Please put in a data string" << endl;
    cin >> buffer;

    data.target = stoi(buffer.substr(0, buffer.find(DATA_DELIMITER)));
    data.posX = stod(buffer.substr(1, buffer.find(DATA_DELIMITER)));
    data.posY = stod(buffer.substr(2, buffer.find(DATA_DELIMITER)));
    data.velX = stod(buffer.substr(3, buffer.find(DATA_DELIMITER)));
    data.velY = stod(buffer.substr(4, buffer.find(DATA_DELIMITER)));
    data.velZ = stod(buffer.substr(5, buffer.find(DATA_DELIMITER)));
    data.accX = stod(buffer.substr(6, buffer.find(DATA_DELIMITER)));
    data.accY = stod(buffer.substr(7, buffer.find(DATA_DELIMITER)));
    data.accZ = stod(buffer.substr(8, buffer.find(DATA_DELIMITER)));

    cout << "Parsed radar data" << endl;
    cout << "Target number: " << data.target << endl;
    cout << "Position Data: X: " << data.posX << " Y: " << data.posY << " Z: " << data.posZ << endl;
    cout << "Velocity Data: X: " << data.velX << " Y: " << data.velY << " Z: " << data.velZ << endl;
    cout << "Acceleration Data: X: " << data.accX << " Y: " << data.accY << " Z: " << data.accZ << endl;
}