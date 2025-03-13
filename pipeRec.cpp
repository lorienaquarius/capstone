#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(){
    std::string pipe_name = "radarpipe";
    mkfifo(pipe_name.c_str(), 0666);
    std::cout << "waiting for messages...";
    int fd = open(pipe_name.c_str(), O_RDONLY);
    if(fd == -1){
        std::cerr << "Pipe error!";
        return 1;
    }

    char buffer[128];
    while (true){
        ssize_t bytesRead = read(fd,buffer,sizeof(buffer)-1);
        if (bytesRead >0){
            buffer[bytesRead] = '\0';
            std::cout << "Pipe output:" << buffer << std::endl;
        }


    }
    close(fd);
    return 0;


}