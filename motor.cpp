//
// Created by sage7 on 2/14/25.
//

#include "motor.h"

#include <iostream>
#include <ostream>
#include <pigpio.h>
#include <unistd.h>
#include <atomic>

using namespace std;
volatile std::atomic<int> ZInt{0};

void encoderZInterrupt(int gpio, int level, uint32_t tick) {
    if (level == 1) {
        ZInt = 1;
    }
}

motor::motor(const int motorNum) {
    this->motorNum = motorNum;
    int result = 0;
    count = 0;

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

    ZInt = 0;

    gpioGlitchFilter(MOTOR_ENCODER_Z_PIN[motorNum], 10);
    gpioSetISRFunc(MOTOR_ENCODER_Z_PIN[motorNum], 1, 100, encoderZInterrupt);

    prev_a = gpioRead(MOTOR_ENCODER_A_PIN[motorNum]);
    prev_b = gpioRead(MOTOR_ENCODER_B_PIN[motorNum]);

}

motor::~motor() {
    gpioWrite(MOTOR_ENABLE_PIN[motorNum], 0);
    gpioWrite(MOTOR_PULSE_PIN[motorNum], direction);
}


/// Calibration function to choose a zero starting position
void motor::calibrate() {
    count = 0;
    cout << "Motor " << motorNum << " calibrated!" << endl;
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
    double targetStepCount = (degrees * STEPS_PER_REVOLUTION/DEGREES_PER_REVOLUTION * GEAR_RATIO); // To work with encoders better, take 2 steps at a time
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
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
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
                gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
                usleep(5);
            }
            usleep(5000);
            // Now slowly rotate to try to find the reset position
            for(int i = 0; i < (STEPS_PER_REVOLUTION/2); i++) {
                gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
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

void motor::turnAbsoluteWrapper(double* degrees) {
    while(1) {
        //cout << "Motor turning " << (direction ? "counterclockwise" : "clockwise") << endl;
        //cout << "Current count: " << count << "\nTarget count: " << targetCount << endl;
        double targetCount = (*degrees * STEPS_PER_REVOLUTION/DEGREES_PER_REVOLUTION * GEAR_RATIO); // To work with encoders better, take 2 steps at a time

        // Turn clockwise
        if(!direction) {
            while(count < targetCount) {

                targetCount = (*degrees * STEPS_PER_REVOLUTION/DEGREES_PER_REVOLUTION * GEAR_RATIO); // To work with encoders better, take 2 steps at a time

                if (!direction != (count < targetCount)) {
                    direction = !direction;
                    gpioWrite(MOTOR_DIRECTION_PIN[motorNum], direction);
                    usleep(6);
                    break;
                }

                gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
                usleep(WORKING_STEP_SPEED);
                curr_a = gpioRead(MOTOR_ENCODER_A_PIN[motorNum]);
                curr_b = gpioRead(MOTOR_ENCODER_B_PIN[motorNum]);

                if(curr_a != prev_a || curr_b != prev_b) {
                    count++;
                }
                prev_a = curr_a;
                prev_b = curr_b;
            }
        }
        // Turn counterclockwise
        else {
            while(count > targetCount) {

                targetCount = (*degrees * STEPS_PER_REVOLUTION/DEGREES_PER_REVOLUTION * GEAR_RATIO); // To work with encoders better, take 2 steps at a time

                if (!direction != (count < targetCount)) {
                    direction = !direction;
                    gpioWrite(MOTOR_DIRECTION_PIN[motorNum], direction);
                    usleep(6);
                    break;
                }

                gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
                usleep(WORKING_STEP_SPEED);
                curr_a = gpioRead(MOTOR_ENCODER_A_PIN[motorNum]);
                curr_b = gpioRead(MOTOR_ENCODER_B_PIN[motorNum]);

                if(curr_a != prev_a || curr_b != prev_b) {
                    count--;
                }
                prev_a = curr_a;
                prev_b = curr_b;
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
        usleep(6);
    }

    //cout << "Motor turning " << (direction ? "counterclockwise" : "clockwise") << endl;
    //cout << "Current count: " << count << "\nTarget count: " << targetCount << endl;

    // Turn clockwise
    if(!direction) {
        while(count < targetCount) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
            usleep(WORKING_STEP_SPEED);
            curr_a = gpioRead(MOTOR_ENCODER_A_PIN[motorNum]);
            curr_b = gpioRead(MOTOR_ENCODER_B_PIN[motorNum]);

            if(curr_a != prev_a || curr_b != prev_b) {
                count++;
            }
            prev_a = curr_a;
            prev_b = curr_b;
        }
    }
    // Turn counterclockwise
    else {
        while(count > targetCount) {
            gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
            usleep(WORKING_STEP_SPEED);
            curr_a = gpioRead(MOTOR_ENCODER_A_PIN[motorNum]);
            curr_b = gpioRead(MOTOR_ENCODER_B_PIN[motorNum]);

            if(curr_a != prev_a || curr_b != prev_b) {
                count--;
            }
            prev_a = curr_a;
            prev_b = curr_b;
        }
    }
    cout << "Turn complete!" << endl;
}

void motor::turnOnce() {
    cout << "Turning the motor one step" << endl;

    cout << "Encoder states before step:" << endl << "A: " << gpioRead(MOTOR_ENCODER_A_PIN[motorNum]) << endl << "B: " << gpioRead(MOTOR_ENCODER_B_PIN[motorNum]) <<endl;
    gpioTrigger(MOTOR_PULSE_PIN[motorNum], WORKING_PULSE_WIDTH, 0);
    usleep(WORKING_ENCODER_STEP_SPEED);
    cout << "Encoder states after step: " << endl << "A: " << gpioRead(MOTOR_ENCODER_A_PIN[motorNum]) << endl << "B: " << gpioRead(MOTOR_ENCODER_B_PIN[motorNum]) <<endl;;
}

void motor::readEncoders() {
    cout << "Current encoder values: " << endl << "A: " << gpioRead(MOTOR_ENCODER_A_PIN[motorNum]) << endl << "B: " << gpioRead(MOTOR_ENCODER_B_PIN[motorNum]) << endl << "Z: " << gpioRead(MOTOR_ENCODER_Z_PIN[motorNum]) << endl;
}

