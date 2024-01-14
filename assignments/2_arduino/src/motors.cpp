#include "motors.hpp"
#include "pin_mappings.h"
#include <Arduino.h>

void setupMotors(bool analogMode) {
    pinMode(LEFT_MOTOR_FORWARD_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_FORWARD_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD_PIN, OUTPUT);

    delay(1000);
    
    if (analogMode)
        turnMotorsAnalog(OFF, OFF, 0, 0);
    else
       turnMotorsDigital(OFF, OFF);
}

void turnOneMotorDigital(uint32_t forwardPin, uint32_t backwardPin, MotorMode mode) {
    switch (mode) {
        case OFF:
            digitalWrite(forwardPin, 0);
            digitalWrite(backwardPin, 0);
            break;
        case FORWARD:
            digitalWrite(backwardPin, 0);
            digitalWrite(forwardPin, 1);
            break;
        case BACKWARD:
            digitalWrite(forwardPin, 0);
            digitalWrite(backwardPin, 1);
            break;
        default:
            printf("Should not happen!");
            exit(1);
    }
}

void turnMotorsDigital(MotorMode leftMotorMode, MotorMode rightMotorMode) {
    turnOneMotorDigital(LEFT_MOTOR_FORWARD_PIN, LEFT_MOTOR_BACKWARD_PIN, leftMotorMode);
    turnOneMotorDigital(RIGHT_MOTOR_FORWARD_PIN, RIGHT_MOTOR_BACKWARD_PIN, rightMotorMode);
}

void turnOneMotorAnalog(uint32_t forwardPin, uint32_t backwardPin, MotorMode mode, uint32_t speed) {
    switch (mode)
    {
    case OFF:
        analogWrite(backwardPin, 0);
        analogWrite(forwardPin, 0);
        break;
    case FORWARD:
        analogWrite(backwardPin, 0);
        analogWrite(forwardPin, speed);
        break;
    case BACKWARD:
        analogWrite(forwardPin, 0);
        analogWrite(backwardPin, speed);
        break;
    default:
        printf("Should not happen!");
        exit(1);
    }
}

void turnMotorsAnalog(MotorMode leftMode, MotorMode rightMode, uint32_t leftSpeed, uint32_t rightSpeed) {
    turnOneMotorAnalog(LEFT_MOTOR_FORWARD_PIN, LEFT_MOTOR_BACKWARD_PIN, leftMode, leftSpeed);
    turnOneMotorAnalog(RIGHT_MOTOR_FORWARD_PIN, RIGHT_MOTOR_BACKWARD_PIN, rightMode, rightSpeed);
}