#include "utils.hpp"
#include "pin_mappings.h"
#include "motors.hpp"
#include "ultrasound_sensors.hpp"
#include "oled_screen.hpp"
#include <Arduino.h> 

enum state {
    STATE_FORWARD,
    STATE_BACKWARD,
    STATE_STOP
};

state currentState;

void transitionToForward() {
    currentState = STATE_FORWARD;
    turnMotorsDigital(FORWARD, FORWARD);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    printTextToOLED("FRONT", 0, true);
}

void transitionToBackward() {
    currentState = STATE_BACKWARD;
    turnMotorsDigital(BACKWARD, BACKWARD);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    printTextToOLED("BACK", 0, true);
}

void transitionToStop() {
    currentState = STATE_STOP;
    turnMotorsDigital(OFF, OFF);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    printTextToOLED("STOP", 0, true);
}

void doStateForward(float distanceCm) {
    if (distanceCm < 15.0f) {
        transitionToStop();
    }
}

void doStateBackward(float distanceCm) {
    if (distanceCm > 10.0f) {
        transitionToStop();
    }
}

void doStateStop(float distanceCm) {
    if (distanceCm > 20.0f) {
        transitionToForward();
    } else if (distanceCm < 5.0f) {
        transitionToBackward();
    }
}

void setup() {
    // Give time for hardware to initialize (solves stupid bug somehow...)
    delay(1000);

    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    setupMotors(false);
    setupUltrasoundSensors();
    setupOLEDScreen();

    printTextToOLED("Hello", 0, true);
    printTextToOLED("World", 1, false);
    delay(2000);

    transitionToForward();
}

void loop() {
    while(true) {
        float distanceCm = getFrontUltrasoundDistanceCm();
        switch (currentState) {
            case STATE_FORWARD:
                doStateForward(distanceCm);
                break;
            case STATE_BACKWARD:
                doStateBackward(distanceCm);
                break;
            case STATE_STOP:
                doStateStop(distanceCm);
                break;
            default:
                printf("Should not hapen!...");
                exit(1);
        }
    }
}
