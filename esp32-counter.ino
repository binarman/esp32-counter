#include "counter_gui.h"

#define i2c_Address 0x3c

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  counter_gui::setup(&display);

  delay(250);
  display.begin(i2c_Address, true);
 
  display.display();
  delay(1000);
  setCpuFrequencyMhz(80);
}

void loop() {
  display.clearDisplay();
  counter_gui::loop();
  display.display();
}
