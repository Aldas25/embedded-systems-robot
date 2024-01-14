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

    setupMotors(true);
    setupOLEDScreen();
    setupIRSensors();
    setupUltrasoundSensors();

    printTextToOLED("Hello", 0, true);
    delay(2000);
    printTextToOLED("World", 1, false);
    delay(2000);

    // printTextToOLED("SPIN", 0, true);
    // turnMotorsDigital(FORWARD, OFF);
    // delay(10000);

    turnMotorsAnalog(FORWARD, FORWARD, 127, 127);
}

bool yellowOn = true;
uint32_t motorSpeed = 255;

void loop() {
    // Slowing motors down
    //printTextToOLED("Speed: " + std::to_string(motorSpeed), 0, true);
   /* turnMotorsAnalog(FORWARD, FORWARD, motorSpeed, motorSpeed);
    if (motorSpeed > 10)
        motorSpeed -= 10;
    else
        motorSpeed = 0;
*/
    // LEDs blinking
    yellowOn = !yellowOn;
    digitalWrite(YELLOW_LED_PIN, yellowOn);
    digitalWrite(RED_LED_PIN, !yellowOn);

    delay(500);
}
