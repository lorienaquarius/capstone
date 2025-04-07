//
// Created by sage7 on 3/13/25.
//

#ifndef RADAR_H
#define RADAR_H
#include <cstdint>

#define DATA_DELIMITER '|'

typedef struct radarData {

    int target;
    double posX;
    double posY;
    double posZ;

    double velX;
    double velY;
    double velZ;

    double accX;
    double accY;
    double accZ;

} radarData;

#endif //RADAR_H

