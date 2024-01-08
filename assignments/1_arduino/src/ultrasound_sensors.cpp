#include "pin_mappings.h"
#include "oled_screen.hpp"
#include <string>
#include <Arduino.h>

void setupUltrasoundSensors() {
    pinMode(FRONT_ULTRASOUND_ECHO_PIN, INPUT);
    pinMode(SIDE_ULTRASOUND_ECHO_PIN, INPUT);
    pinMode(FRONT_ULTRASOUND_TRIG_PIN, OUTPUT);
    pinMode(SIDE_ULTRASOUND_TRIG_PIN, OUTPUT);
}

void printUltrasoundInfo() {
    digitalWrite(FRONT_ULTRASOUND_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(FRONT_ULTRASOUND_TRIG_PIN, LOW);

    uint32_t duration_us = pulseIn(FRONT_ULTRASOUND_ECHO_PIN, HIGH);
    float distance_cm = (float) duration_us * 0.017f;

    //int precision_str = 2;
    //std::string rounded_distance_str = std::to_string(std::round(distance_cm * std::pow(10, precision_str)) / std::pow(10, precision_str)).substr(0, std::to_string(std::round(distance_cm * std::pow(10, precision_str)) / std::pow(10, precision_str)).find(".") + precision_str + 1);
    std::string rounded_distance_str = "0000";
    printTextToOLED("F US: " + std::to_string(duration_us), 0, true);
    printTextToOLED("      " + rounded_distance_str + " cm", 1, false);
}