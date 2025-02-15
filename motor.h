//
// Created by sage7 on 2/14/25.
//

#ifndef MOTOR_H
#define MOTOR_H

#define STEPS_PER_REVOLUTION 400
#define DEGREES_PER_REVOLUTION 360
#define GEAR_RATIO (19.0 + 35.0/157.0)

class motor {

public:
    explicit motor(int motorNum);
    ~motor();
    void turnRelative(double degrees);
    void turnAbsolute(double degrees);
    void calibrate();
private:
    inline void turn(double targetCount);
    int count = 0;
    bool direction = false;
    int motorNum;
    const int MOTOR_DIRECTION_PIN[2] = {8, 0};
    const int MOTOR_ENABLE_PIN[2] = {7, 0};
    const int MOTOR_PULSE_PIN[2] = {12, 0};
    const int MOTOR_ENCODER_A_PIN[2] = {26, 0};
    const int MOTOR_ENCODER_B_PIN[2] = {19, 0};
    const int MOTOR_ENCODER_Z_PIN[2] = {13, 0};
};

#endif //MOTOR_H
