enum MotorMode {
    OFF,
    FORWARD,
    BACKWARD
};

void setupMotors();
void turnMotorsDigital(MotorMode leftMotorMode, MotorMode rightMotorMode);