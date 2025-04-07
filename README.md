# SYSC4907 Group 8: Detection of Unlawful Drones  September 2024 - April 2025  By Lorien Aquarius, Liam Burns, Amin Zeina, and Aniesh Sabnani 

## Setup and Running
This project was setup using CLion, which uses cmake. Project is run using
1. `$cmake .`
2. `$make`
3. `$sudo ./capstone`

Also, it is common to need to run `$sudo killall pigpiod` when the raspberry pi is first started, in order for the motor process to control the gpio 
## Organization
This code is organized under 3 main files: main.cpp, motor.cpp, and motor.h
The main file is responsible for the main loop of calibrating, and then retrieving information from the radar pipe and sending it to the motors in their respective threads
the motor.cpp file is responsible for the low level turning, calculating and keeping track of steps, and other low level hardware control
motor.h keeps track of constants, and also helps define the motor class. This is where you will also find pinout of the various motor control pins like pulse, enable, direction

## Calibration
Calibration should be done such that the camera is parallel to the ground, and in line with the radar. Upon starting the program, the first thing the camera will do is
point slightly downwards to look at the radar

## Synchronization with the radar program
the radar python wrapper should be run after the motor program, or else the python wrapper will not be able to open the pipe properly
see [here](https://github.com/burnsy2830/ProjectStarshotRadarWrapper) for the python wrapper to grab data from the radar
