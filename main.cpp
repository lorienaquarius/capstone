#include <pigpio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "motor.h"


#define MICROSTEPS = 2

using namespace std;

void printMenu() {
    cout << "Main menu" << endl;
    cout << "\ttr: Turn X degrees relative to the current position" << endl;
    cout << "\tta: Turn X degrees relative to the calibrated 0 position" << endl;
    cout << "\tm: print menu again" << endl;
    cout << "\tq: Quit" << endl;
}

enum stringCodes {
    tr,
    ta,
    m,
    q,
    d
};

stringCodes hasher(string in) {
    if(in == "tr") return tr;
    else if (in == "ta") return ta;
    else if (in == "m") return m;
    else if (in == "q") return q;
    else return d;
}

int main(int argc, char *argv[]) {

    if(gpioInitialise() < 0) {
        cout << "Failed to initialize GPIO" << endl;
        exit(1);
    }

    cout << "Welcome to the stepper motor turning application, let's start by calibrating a motor!" << endl;
    sleep(1);
    cout << "The motor will start by turning clockwise to the nearest index position" << endl;
    sleep(1);

    string degrees;
    auto* motor0 = new motor(0);
    motor0->calibrate();

    string input;
    printMenu();
    do {

        cin >> input;
        switch(hasher(input)) {
            case tr:
                cout << "Please enter number of degrees" << endl;
                cin >> degrees;
                motor0->turnRelative(stod(degrees));
                break;
            case ta:
                cout << "Please enter number of degrees" << endl;
                cin >> degrees;
                motor0->turnAbsolute(stod(degrees));
                break;
            case m:
                printMenu();
                break;
            case q:
                break;
            default:
                cout << "Please select a valid menu option" << endl;
        }

    } while (input != "q");

    delete motor0;

}
