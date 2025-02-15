//
// Created by sage7 on 2/14/25.
//

#include "motor.h"

#include <iostream>
#include <ostream>
#include <pigpio.h>
#include <unistd.h>

using namespace std;
motor::motor(const int motorNum) {
    this->motorNum = motorNum;
    int result = 0;

    result |= gpioSetMode(MOTOR_DIRECTION_PIN[motorNum], PI_OUTPUT);
    result |= gpioSetMode(MOTOR_ENABLE_PIN[motorNum], PI_OUTPUT);
    result |= gpioSetMode(MOTOR_PULSE_PIN[motorNum], PI_OUTPUT);

    result |= gpioSetMode(MOTOR_ENCODER_A_PIN[motorNum], PI_INPUT);
    result |= gpioSetMode(MOTOR_ENCODER_B_PIN[motorNum], PI_INPUT);
    result |= gpioSetMode(MOTOR_ENCODER_Z_PIN[motorNum], PI_INPUT);

    if (result > 0) {
        cout << "Failed to set GPIO Modes" << endl;
        exit(1);
    }

    gpioWrite(MOTOR_ENABLE_PIN[motorNum], 1);
    gpioWrite(MOTOR_DIRECTION_PIN[motorNum], direction);
    gpioWrite(MOTOR_PULSE_PIN[motorNum], 1);

}

motor::~motor() {
    gpioWrite(MOTOR_ENABLE_PIN[motorNum], 0);
    gpioWrite(MOTOR_PULSE_PIN[motorNum], direction);
}


/// Calibration function to choose a zero starting position
void motor::calibrate() {
    cout << "calibrating motor: "<< motorNum << endl;
    string decision;
    do {
        while(gpioRead(MOTOR_ENCODER_Z_PIN[motorNum]) < 1) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
            usleep(4000);
        }
        cout << "Would you like to set the current position as the zero position? [y/n]" << endl;
        cin >> decision;
    } while(decision != "y" && decision != "Y");
    count = 0;
    cout << "Motor calibrated!" << endl;

}

/// Turn the motor a set amount of degrees from the current positiion
/// @param degrees How many degrees to turn from the current position. Positive for clockwise, negative for counterclockwise
void motor::turnRelative(double degrees) {

    double targetStepCount = (degrees * STEPS_PER_REVOLUTION/DEGREES_PER_REVOLUTION * GEAR_RATIO) + count;
    turn(targetStepCount);
}

/// Function to turn to a certain angle relative to the selected 0 positiion
/// @param degrees How many degrees to turn from the calibrated 0 position
void motor::turnAbsolute(double degrees) {
    double targetStepCount = (degrees * STEPS_PER_REVOLUTION/DEGREES_PER_REVOLUTION * GEAR_RATIO);
    turn(targetStepCount);
}


/// Helper function to turn the motor to a generic encoder count
/// @param targetCount the target step count
inline void motor::turn(double targetCount) {

    if (!direction != (count < targetCount)) {
        direction = !direction;
        gpioWrite(MOTOR_DIRECTION_PIN[motorNum], direction);
    }

    // Turn Forward
    if(direction) {
        while(count < (targetCount)) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
            usleep(5);
            count++;
        }
    }
    // Turn Backwards
    else {
        while(count > (targetCount)) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
            usleep(5);
            count--;
        }
    }
}