#include "infrared_sensors.hpp"
#include "oled_screen.hpp"
#include "pin_mappings.h"
#include <string>
#include <Arduino.h>

#define LEFT_IR_THRESHOLD 200
#define RIGHT_IR_THRESHOLD 200

void setupIRSensors() {
    pinMode(LEFT_INFRARED_PIN, INPUT);
    pinMode(RIGHT_INFRARED_PIN, INPUT);
}

void printIRSensorInfoToScreen() {
    // Infrared sensors
    uint32_t analogIRSensorLeft = analogRead(LEFT_INFRARED_PIN);
    int digitalIRSenorLeft = digitalRead(LEFT_INFRARED_PIN);
    uint32_t analogIRSensorRight = analogRead(RIGHT_INFRARED_PIN);
    int digitalIRSensorRight = digitalRead(RIGHT_INFRARED_PIN);

    bool leftValue = getLeftIRSensorValue();
    bool rightValue = getRightIRSensorValue();

    printTextToOLED("L IR: A: " + std::to_string(analogIRSensorLeft) + " (" + std::to_string(leftValue) + ")", 
                    0, true);
    printTextToOLED("      D: " + std::to_string(digitalIRSenorLeft), 1, false);
    printTextToOLED("R IR: A: " + std::to_string(analogIRSensorRight) + " (" + std::to_string(rightValue) + ")", 
                    2, false);
    printTextToOLED("      D: " + std::to_string(digitalIRSensorRight), 3, false);
}

bool getLeftIRSensorValue() {
    return analogRead(LEFT_INFRARED_PIN) > LEFT_IR_THRESHOLD;
}

bool getRightIRSensorValue() {
    return analogRead(RIGHT_INFRARED_PIN) > RIGHT_IR_THRESHOLD;
}