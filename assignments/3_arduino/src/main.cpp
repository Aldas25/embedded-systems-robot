#include "utils.hpp"
#include "pin_mappings.h"
#include "motors.hpp"
#include "oled_screen.hpp"
#include "infrared_sensors.hpp"
#include <Arduino.h> 

void setup() {
    // Give time for hardware to initialize (solves stupid bug somehow...)
    delay(1000);

    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    setupMotors(false);
    setupIRSensors();
    setupOLEDScreen();

    printTextToOLED("Hello", 0, true);
    printTextToOLED("World", 1, false);
    printTextToOLED("3. Line Follow", 2, false);
    delay(2000);
}

int waitForDisplay = 0;

void loop() {
    if (waitForDisplay <= 0) {
        printIRSensorInfoToScreen();
        waitForDisplay = 100;
    }
    waitForDisplay--;

    bool irLeft = getLeftIRSensorValue();
    bool irRight = getRightIRSensorValue();

    MotorMode leftMode = irLeft ? OFF : FORWARD;
    MotorMode rightMode = irRight ? OFF : FORWARD;

    turnMotorsDigital(leftMode, rightMode);
    digitalWrite(YELLOW_LED_PIN, irLeft);
    digitalWrite(RED_LED_PIN, irRight);
}
