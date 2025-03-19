//
// Created by sage7 on 3/19/25.
//

#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <mutex>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

#include "radar.h"
#include "PipeRec.h"
using namespace std;

///
/// @param dataString The string containing the data to be parsed
/// @param data the radarData object to put data into
void parseRadarData(string dataString, radarData* data, mutex* radarMutex) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = dataString.find(DATA_DELIMITER)) != std::string::npos) {
        token = dataString.substr(0, pos);
        tokens.push_back(token);
        dataString.erase(0, pos + 1);
    }
    tokens.push_back(dataString);
    radarMutex->lock();
    data->target = stoi(tokens[0]);
    data->posX = stod(tokens[1]);
    data->posY = stod(tokens[2]);
    data->posZ = stod(tokens[3]);
    data->velX = stod(tokens[4]);
    data->velY = stod(tokens[5]);
    data->velZ = stod(tokens[6]);
    data->accX = stod(tokens[7]);
    data->accY = stod(tokens[8]);
    data->accZ = stod(tokens[9]);
    radarMutex->unlock();

}

void readData(radarData* data, mutex* radarMutex){
    string pipe_name = "radarpipe";
    mkfifo(pipe_name.c_str(), 0666);
    cout << "waiting for messages...";
    int fd = open(pipe_name.c_str(), O_RDONLY);
    if(fd == -1){
        std::cerr << "Pipe error!";
        return;
    }

    char buffer_raw[128];
    while(true) {
        ssize_t bytesRead = read(fd,buffer_raw,sizeof(buffer_raw)-1);
        if (bytesRead >0) {
            string buffer(buffer_raw);
            parseRadarData(buffer,  data, radarMutex);
        }
    }


    // cout << "Parsed radar data" << endl;
    // cout << "Target number: " << data.target << endl;
    // cout << "Position Data: X: " << data.posX << " Y: " << data.posY << " Z: " << data.posZ << endl;
    // cout << "Velocity Data: X: " << data.velX << " Y: " << data.velY << " Z: " << data.velZ << endl;
    // cout << "Acceleration Data: X: " << data.accX << " Y: " << data.accY << " Z: " << data.accZ << endl;
}