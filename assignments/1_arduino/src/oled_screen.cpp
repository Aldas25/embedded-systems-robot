#include "oled_screen.hpp"
#include <string>

// Arduino libraries for OLED screen
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_ADDRESS 0x3C

const int totalLineCount = 4;
Adafruit_SSD1306 display;
bool displayInitialized = false;
std::string textInLines[totalLineCount];

void setupOLEDScreen() {
    for (int line = 0; line < totalLineCount; line++) {
        textInLines[line] = "";
    }

    displayInitialized = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    if (!displayInitialized) {
        return;
    }

    display.clearDisplay();
}

void printAllLines(bool clearScreen=true) {
    if (clearScreen) {
        display.clearDisplay();
    }

    // Text settings
    display.setTextSize(1);
    display.setTextColor(WHITE);

    for (int line = 0; line < totalLineCount; line++) {
        display.setCursor(0, 8 * line);
        display.println(textInLines[line].c_str());
    }

    display.display();
}

void printTextToOLED(std::string text, int lineNumber, bool clearScreen) {
    if (!displayInitialized) {
        return;
    }

    textInLines[lineNumber] = text;
    printAllLines(clearScreen);
}

