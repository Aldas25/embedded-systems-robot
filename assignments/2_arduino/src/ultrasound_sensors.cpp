#include "pin_mappings.h"
#include "oled_screen.hpp"
#include <string>
#include <Arduino.h>

void setupUltrasoundSensors() {
    pinMode(FRONT_ULTRASOUND_ECHO_PIN, INPUT);
   // pinMode(SIDE_ULTRASOUND_ECHO_PIN, INPUT);
    pinMode(FRONT_ULTRASOUND_TRIG_PIN, OUTPUT);
   // pinMode(SIDE_ULTRASOUND_TRIG_PIN, OUTPUT);
}

float getUltrasoundDistanceCm(uint32_t trigPin, uint32_t echoPin) {
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    uint32_t duration_us = pulseIn(echoPin, HIGH);
    float distance_cm = (float) duration_us * 0.017f;
    return distance_cm;
}

float getFrontUltrasoundDistanceCm() {
    return getUltrasoundDistanceCm(FRONT_ULTRASOUND_TRIG_PIN, FRONT_ULTRASOUND_ECHO_PIN);
}

float getSideUltrasoundDistanceCm() {
    return getUltrasoundDistanceCm(SIDE_ULTRASOUND_TRIG_PIN, SIDE_ULTRASOUND_ECHO_PIN);
}

std::string floatToStr(float f) {
    int before_comma = (int) f;
    int after_comma = (int) (f * 100.0f) - (100 * f);

    return std::to_string(before_comma) + "." + (after_comma < 10 ? "0" : "") + std::to_string(after_comma);
}

void printUltrasoundInfo() {
    float frontDistCm = getFrontUltrasoundDistanceCm();
    float sideDistCm = getSideUltrasoundDistanceCm();

    printTextToOLED("Front US: " + floatToStr(frontDistCm) + " cm", 0, true);
    printTextToOLED(" Side US: " + floatToStr(sideDistCm) + " cm", 1, false);
}