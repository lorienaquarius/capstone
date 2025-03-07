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
            cout << "Encoder Z state: " << gpioRead(MOTOR_ENCODER_Z_PIN[motorNum]) << endl;
            usleep(5000);
        }
        cout << "Would you like to set the current position as the zero position? [y/n]" << endl;
        cin >> decision;
    } while(decision != "y" && decision != "Y");
    count = 0;
    cout << "Motor calibrated!" << endl;

}

/// Turn the motor a set amount of degrees from the current position
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

void motor::reset() {
    turn(0);
    usleep(5000);

    // Went back to count 0, but the index encoder is still low
    if(gpioRead(MOTOR_ENCODER_Z_PIN[motorNum]) == 0) {
        bool found = false;
        cout << "Slight motor drift detected, going to closest reset point" << endl;

        // Reset position was undershot, should be somewhere close. Try moving half a revolution, see if we can find it
        for(int i = 0; i < (STEPS_PER_REVOLUTION/2); i++) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
            usleep(5000);
            if(gpioRead(MOTOR_ENCODER_Z_PIN[motorNum]) == 1) {
                cout << "Reset position found" << endl;
                found = true;
                break;
            }
        }
        // We overshot the reset position to begin with, try resetting the other way
        if(!found) {

            cout << "Looked the wrong way, trying to reset again" << endl;
            direction = !direction;
            gpioWrite(MOTOR_DIRECTION_PIN[motorNum], direction);

            // Go back to our initial reset position quickly
            for(int i = 0; i < (STEPS_PER_REVOLUTION/2); i++) {
                gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
                usleep(5);
            }
            usleep(5000);
            // Now slowly rotate to try to find the reset position
            for(int i = 0; i < (STEPS_PER_REVOLUTION/2); i++) {
                gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
                usleep(5000);
                if(gpioRead(MOTOR_ENCODER_Z_PIN[motorNum]) == 1) {
                    found = true;
                    break;
                }
            }

            // We tried half a reset position in each direction and still didn't find it, you're just fucked
            if(!found) {
                cout << "Reset failed, please recalibrate motor" << endl;
            }
        }

    }
}
/// Helper function to turn the motor to a generic encoder count
/// @param targetCount the target step count
inline void motor::turn(double targetCount) {

    if (!direction != (count < targetCount)) {
        direction = !direction;
        gpioWrite(MOTOR_DIRECTION_PIN[motorNum], direction);
    }

    cout << "Motor turning " << (direction ? "counterclockwise" : "clockwise") << endl;
    cout << "Current count: " << count << "\nTarget count: " << targetCount << endl;

    // Turn clockwise
    if(!direction) {
        while(count < (targetCount)) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
            usleep(5);
            count++;
        }
    }
    // Turn counterclockwise
    else {
        while(count > (targetCount)) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], 8, 0);
            usleep(5);
            count--;
        }
    }
    cout << "Turn complete!" << endl;
}