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
    string pipe_name = "radarpipe";
    mkfifo(pipe_name.c_str(), 0666);
    cout << "waiting for messages...";
    int fd = open(pipe_name.c_str(), O_RDONLY);
    if(fd == -1){
        std::cerr << "Pipe error!";
        return 1;
    }

    char buffer[128];
    radarData data;
    while (true){
        ssize_t bytesRead = read(fd,buffer,sizeof(buffer)-1);
        if (bytesRead >0){
            buffer[bytesRead] = '\0';
            data.target = regex_match(buffer, regex("(?:Target )[0-9]"));
            data.posX = regex_search(buffer, regex("(?:pos=\()[0-9]*\.[0-9]*"));
            data.posY = regex_search(buffer, regex("(?:pos=\([0-9]*\.[0-9]*,) [0-9]*\.[0-9]*"));
            data.posZ = regex_search(buffer, regex("(?:pos=\(([0-9]*\.[0-9]*, ){2})[0-9]*\.[0-9]*"));
            data.velX = regex_search(buffer, regex("(?:vel=\()[0-9]*\.[0-9]*"));
            data.velY = regex_search(buffer, regex("(?:vel=\([0-9]*\.[0-9]*,) [0-9]*\.[0-9]*"));
            data.velZ = regex_search(buffer, regex("(?:vel=\(([0-9]*\.[0-9]*, ){2})[0-9]*\.[0-9]*"));
            data.accX = regex_search(buffer, regex("(?:acc=\()[0-9]*\.[0-9]*"));
            data.accY = regex_search(buffer, regex("(?:acc=\(-?[0-9]*\.[0-9]*, )-?[0-9]*\.[0-9]*"));
            data.accZ = regex_search(buffer, regex("(?:acc=\((-?[0-9]*\.[0-9]*, ){2})-?[0-9]*\.[0-9]*"));


        }
    }
    close(fd);
    return 0;
}