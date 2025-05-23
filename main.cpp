#include <atomic>
#include <fcntl.h>
#include <pigpio.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>
#include <math.h>
#include <cmath>
#include <thread>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

#include "motor.h"
#include "radar.h"

#define MICROSTEPS = 2

using namespace std;

///
/// @param dataString The string containing the data to be parsed
/// @param data the radarData object to put data into
void parseRadarData(string dataString, radarData* data, mutex* radarMutex) {
    vector<std::string> tokens;
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

    //cout << "Got position X: " << data->posX << " Y: " << data->posY << " Z: " << data->posZ << endl;
    radarMutex->unlock();


}

void readData(radarData* data, mutex* radarMutex){
    string pipe_name = "../radarpipe";
    mkfifo(pipe_name.c_str(), 0666);
    //cout << "waiting for messages...";
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



void printMenu() {
    cout << "Main menu" << endl;
    cout << "\tr: Reset motor" << endl;
    cout << "\tc: Calibrate motor manually" << endl;
    cout << "\tto: Turn one step" << endl;
    cout << "\ts: start detection program" << endl;
    cout << "\tq: Quit" << endl << ">>> ";
}

enum stringCodes {
    tr,
    ta,
    to,
    m,
    r,
    q,
    c,
    d,
    re,
    s
};

stringCodes hasher(string* in) {
    if(*in == "tr") return tr;
    else if (*in == "ta") return ta;
    else if (*in == "m") return m;
    else if (*in == "q") return q;
    else if (*in == "r") return r;
    else if (*in == "c") return c;
    else if (*in == "to") return to;
    else if (*in == "re") return re;
    else if (*in == "s") return s;
    else return d;
}

int main(int argc, char *argv[]) {

    if(gpioInitialise() < 0) {
        cout << "Failed to initialize GPIO" << endl;
        exit(1);
    }
    radarData radarInfo;
    mutex radarDataMutex;

    string degrees;
    motor motor0(0);
    motor motor1(1);



    string input;
    do {
        printMenu();
        cin >> input;
        switch(hasher(&input)) {
            case m:
                printMenu();
                break;
            case r:
                cout << "Attempting reset" << endl;
                motor0.reset();
                break;
            case c:
                cout << "Entering Calibration menu" << endl;
                cout << "For reference, motor 0 controls the base, motor 1 controls the arm" << endl;
                {
                    string calibrationInput;
                    while(calibrationInput != "q") {
                        cout << "Please enter which motor to turn" << endl << ">>> ";
                        cin >> calibrationInput;
                        if (calibrationInput == "q") {
                            break;
                        }
                        int motorNumber = stoi(calibrationInput); // motor number should be the first thing
                        cout << "Please enter the number of degrees to turn" << endl << ">>> ";
                        cin >> calibrationInput;
                        if (calibrationInput == "q") {
                            break;
                        }
                        double degrees = stod(calibrationInput); // degrees to turn should be the rest of the text
                        cout << "Turning motor number " << motorNumber << " " << degrees << " degrees" << endl;
                        if(motorNumber) {
                            motor1.turnRelative(degrees);
                        } else {
                            motor0.turnRelative(degrees);
                        }
                    }
                    motor0.calibrate();
                    motor1.calibrate();
                }
                break;
            case q:
                break;
            case to:
                cout << "Turning once" << endl;
                motor0.turnOnce();
                break;
            case re:
                motor0.readEncoders();
                break;
            default:
                cout << "Please select a valid menu option" << endl << ">>> ";
        }

    } while (input != "q" && input != "s");
    if(input == "q") {
        cout << "quitting program" << endl;
        exit(0);
    }

    thread pipeThread(readData, &radarInfo, &radarDataMutex);
    double motor0Angle = 0;
    double motor1Angle = 0;
    double prevMotor0Angle = 0;
    double prevMotor1Angle = 0;

    double turnAngle0 = 0;
    double turnAngle1 = 0;

    double radarX = 0;
    double radarY = 0;
    double radarZ = 0;

    bool localUpdated = false;

    // thread turn1Thread(&motor::turnAbsoluteWrapper, &motor1, &turnAngle1);
    // thread turn0Thread(&motor::turnAbsoluteWrapper, &motor0, &turnAngle0);

    while(1) {

        radarDataMutex.lock();
        // There is an axis transformation from the radar to the camera, which is why the coordinates are a bit shuffled
        radarX = radarInfo.posZ + 0.347; // Measured offsets from the radar to the camera
        radarY = radarInfo.posY - 0.161; // Measured offset from the radar to the camera
        radarZ = -radarInfo.posX;
        radarDataMutex.unlock();

        motor0Angle = atan2(radarZ , radarX) * 180.0 / M_PI;
        motor1Angle = atan(radarY / sqrt(pow(radarX, 2) + pow(radarY, 2))) * 180.0 / M_PI;
         // cout << "Calculated pan of: " << motor0Angle << " and tilt: " << motor1Angle << endl;

        // Denoising conditions. Only update the angle to turn to if it's a real update
        if(radarX <= 0) {
            localUpdated = false;
            cout << "Detected a negative X value" << endl;
        } else if(motor0Angle > 90 || motor0Angle < -90) {
            localUpdated = false;
            cout << "Detected too large of an angle" << endl;
        } else if((abs(motor0Angle - prevMotor0Angle) > 20)) {
            localUpdated = false;
            cout << "Change in angle was too great, ignoring" << endl;
        } else if(prevMotor0Angle == motor0Angle && prevMotor1Angle == motor1Angle) {
            localUpdated = false;
        } else {
            localUpdated = true;
        }

        // Update the turning variables
        if(localUpdated) {
            cout << "Updating angle to pan: " << motor0Angle << " tilt: " << motor1Angle << endl;
            turnAngle0 = motor0Angle;
            turnAngle1 = motor1Angle;
            thread turn1Thread(&motor::turnAbsolute, &motor1, turnAngle1);
            thread turn0Thread(&motor::turnAbsolute, &motor0, turnAngle0);
            turn1Thread.join();
            turn0Thread.join();
        }

        // Update previous angles
        prevMotor0Angle = motor0Angle;
        prevMotor1Angle = motor1Angle;

        usleep(1000);

    }

}
