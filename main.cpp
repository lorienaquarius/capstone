#include <pigpio.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>
#include <math.h>
#include <thread>
#include <unistd.h>
#include "motor.h"
#include "radar.h"
#include "PipeRec.h"

#define MICROSTEPS = 2

using namespace std;

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

    string degrees;
    motor motor0(0);
    motor motor1(1);

    radarData radarInfo;
    mutex radarDataMutex;

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
                    while(input != "q") {
                        cout << "Please enter which motor to turn and number of degrees" << endl << ">>> ";
                        cin >> input;
                        int motorNumber = stoi(calibrationInput.substr(0, 1)); // motor number should be the first thing
                        double degrees = stod(calibrationInput.substr(2, input.length() - 2)); // degrees to turn should be the rest of the text
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

    } while (input != "q" || input != "s");
    if(input == "q") {
        cout << "quitting program" << endl;
        exit(0);
    }

    thread pipeThread(readData, &radarInfo, &radarDataMutex);
    double motor0Angle;
    double motor1Angle;

    double radarX;
    double radarY;
    double radarZ;

    while(1) {
        radarDataMutex.lock();


        // There is an axis transformation from the radar to the camera, which is why the coordinates are a bit shuffled
        radarX = radarInfo.posZ + 347; // Measured offsets from the radar to the camera
        radarY = radarInfo.posY - 161; // Measured offset from the radar to the camera
        radarZ = -radarInfo.posX;
        radarDataMutex.unlock();

        motor0Angle = tan(radarZ / radarX) * 180 / M_PI;
        motor1Angle = tan(radarY / sqrt(pow(radarX, 2) + pow(radarY, 2))) * 180 / M_PI;

        // Get Axes turning at the same time
        thread turnThread(&motor::turnAbsolute, &motor1, motor1Angle);
        motor0.turnAbsolute(motor0Angle);

        // Make sure both are finished before reading again
        turnThread.join();
    }


}
