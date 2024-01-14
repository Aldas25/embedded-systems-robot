#include <stdint.h>

enum MotorMode {
    OFF,
    FORWARD,
    BACKWARD
};

void setupMotors(bool analogMode);
void turnMotorsDigital(MotorMode leftMotorMode, MotorMode rightMotorMode);
void turnMotorsAnalog(MotorMode leftMode, MotorMode rightMode, uint32_t leftSpeed, uint32_t rightSpeed);