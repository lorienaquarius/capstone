#include <pigpio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

#define MOTOR_ONE_DIRECTION_PIN 8
#define MOTOR_ONE_ENABLE_PIN 7
#define MOTOR_ONE_PULSE_PIN 12
#define MOTOR_ONE_ENCODER_A_PIN 26
#define MOTOR_ONE_ENCODER_B_PIN 19
#define MOTOR_ONE_ENCODER_Z_PIN 13

#define STEPS_PER_REVOLUTION 3844.585987
#define MICROSTEPS = 2

using namespace std;

int main(int argc, char *argv[]) {
    if(argc < 2) {
        cout << "Incorrect usage. Please enter degrees to turn to" << endl;
        exit(1);
    }
    string arg = argv[1];
    const int ENCODER_INCREMENTS[16] = {0, -1, 1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
    int currentAB = 0;
    int previousAB = 0;
    int phaseA;
    int phaseB;
    int phaseZ;

    if(gpioInitialise() < 0) {
        cout << "Failed to initialize GPIO" << endl;
        exit(1);
    }

    int result = 0;

    result |= gpioSetMode(MOTOR_ONE_DIRECTION_PIN, PI_OUTPUT);
    result |= gpioSetMode(MOTOR_ONE_ENABLE_PIN, PI_OUTPUT);
    result |= gpioSetMode(MOTOR_ONE_PULSE_PIN, PI_OUTPUT);

    result |= gpioSetMode(MOTOR_ONE_ENCODER_A_PIN, PI_INPUT);
    result |= gpioSetMode(MOTOR_ONE_ENCODER_B_PIN, PI_INPUT);
    result |= gpioSetMode(MOTOR_ONE_ENCODER_Z_PIN, PI_INPUT);

    if (result > 0) {
        cout << "Failed to set GPIO Modes" << endl;
        exit(1);
    }

    int stepCount = 0;

    double targetStepCount = stof(arg) * (400.0/360.0) * 19.2223;
    cout <<"Target encoder count: " << targetStepCount << endl;

    // phaseA = gpioRead(MOTOR_ONE_ENCODER_A_PIN);
    // phaseB = gpioRead(MOTOR_ONE_ENCODER_B_PIN);
    // phaseZ = gpioRead(MOTOR_ONE_ENCODER_Z_PIN);
    // previousAB = (phaseA << 1) | phaseB;

    gpioWrite(MOTOR_ONE_ENABLE_PIN, 1);
    usleep(500);
    gpioWrite(MOTOR_ONE_DIRECTION_PIN, 0);
    usleep(5);
    gpioWrite(MOTOR_ONE_PULSE_PIN, 1);
    while (stepCount < targetStepCount) {
        gpioTrigger(MOTOR_ONE_PULSE_PIN, 8, 0);
        usleep(1000);

        stepCount++;

        // phaseA = gpioRead(MOTOR_ONE_ENCODER_A_PIN);
        // phaseB = gpioRead(MOTOR_ONE_ENCODER_B_PIN);
        // phaseZ = gpioRead(MOTOR_ONE_ENCODER_Z_PIN);
        // currentAB = (phaseA << 1) + phaseB;
        // encoderCount += ENCODER_INCREMENTS[(previousAB << 2) | currentAB];
        // previousAB = currentAB;
        // cout << "Current encoder count: " << encoderCount << endl;
        // if(i > 1000) break;

    }
    cout << "Current step count: " << stepCount << endl;
    gpioWrite(MOTOR_ONE_ENABLE_PIN, 0);
    gpioWrite(MOTOR_ONE_DIRECTION_PIN, 0);
}
