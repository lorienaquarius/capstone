//
// Created by sage7 on 2/14/25.
//

#ifndef MOTOR_H
#define MOTOR_H

#define STEPS_PER_REVOLUTION 400
#define DEGREES_PER_REVOLUTION 360
#define GEAR_RATIO (19.0 + 35.0/157.0)
#define MIN_PULSE_WIDTH 8
#define WORKING_PULSE_WIDTH 100
#define MAX_STEP_SPEED 8
#define WORKING_STEP_SPEED 1000 // delay between steps in microseconds, while normal spinning
#define WORKING_ENCODER_STEP_SPEED 1000 // Delay between steps in microseconds, while tryiing to use an encoder
#include <pigpio.h>

class motor {

public:

    explicit motor(int motorNum);
    ~motor();
    void turnRelative(double degrees);
    void turnAbsolute(double degrees);
    void calibrate();


    void reset();
    void turnOnce();
    void readEncoders();

private:
    void turn(double targetCount);
    int count = 0;
    bool direction = false;
    int motorNum;
    bool prev_a;
    bool prev_b;
    bool curr_a;
    bool curr_b;

    /* ***********************************************
     * TOP MOTOR IS MOTOR 1, BOTTOM MOTOR IS MOTOR 0 *
     ************************************************/
    const int MOTOR_DIRECTION_PIN[2] = {28, 3};
    const int MOTOR_ENABLE_PIN[2] = {16, 4};
    const int MOTOR_PULSE_PIN[2] = {21, 2};
    const int MOTOR_ENCODER_A_PIN[2] = {13, 12};
    const int MOTOR_ENCODER_B_PIN[2] = {6, 5};
    const int MOTOR_ENCODER_Z_PIN[2] = {26, 22};
};

#endif //MOTOR_H
