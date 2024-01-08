#include "utils.hpp"
#include "pin_mappings.h"
#include "motors.hpp"
#include "oled_screen.hpp"
#include "infrared_sensors.hpp"
#include "ultrasound_sensors.hpp"
#include <Arduino.h> 

void setup() {
    // Give time for hardware to initialize (solves stupid bug somehow...)
    delay(1000);

    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    setupMotors();
    setupOLEDScreen();
    setupIRSensors();
    setupUltrasoundSensors();

    printTextToOLED("Hello", 0, true);
    delay(2000);
    printTextToOLED("World", 1, false);
    delay(2000);
}

bool yellowOn = true;

void loop() {
    printUltrasoundInfo();

    // LEDs blinking
    yellowOn = !yellowOn;
    digitalWrite(YELLOW_LED_PIN, yellowOn);
    digitalWrite(RED_LED_PIN, !yellowOn);
    delay(500);
}
