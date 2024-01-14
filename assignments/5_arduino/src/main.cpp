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

    pinMode(RIGHT_SPEED_ENCODER_PIN, INPUT);
    pinMode(LEFT_SPEED_ENCODER_PIN, INPUT);

    setupMotors(false);
    setupOLEDScreen();

    printTextToOLED("Hello", 0, true);
    printTextToOLED("World", 1, false);
    printTextToOLED("5. Test Speed encoders", 2, false);
    delay(2000);

    turnMotorsDigital(FORWARD, FORWARD);
}

bool yellowOn = true;
int lastDig = 0;
constexpr int loopMax = 1000 * 1000;
int printLoop = loopMax;
int cnt = 0;

void loop() {

    // TODO
    // 139 cm = about 300 counts

    int dig = digitalRead(LEFT_SPEED_ENCODER_PIN);
    if (dig != lastDig) {
        lastDig = dig;
        cnt++;
    }

    printLoop--;
    if (printLoop <= 0) {
        printLoop = loopMax;
        digitalWrite(YELLOW_LED_PIN, yellowOn);
        yellowOn = !yellowOn;

        printTextToOLED("Cnt: " + std::to_string(cnt), 0, true);
    }
}
