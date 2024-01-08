#include "utils.hpp"
#include "pin_mappings.h"
#include "motors.hpp"
#include <Arduino.h> 

void setup() {
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    setupMotors();

    turnMotorsDigital(FORWARD, FORWARD);
}

bool yellowOn = true;

void loop() {
    yellowOn = !yellowOn;
    digitalWrite(YELLOW_LED_PIN, yellowOn);
    digitalWrite(RED_LED_PIN, !yellowOn);
    delay(500);
}
