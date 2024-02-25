#include "counter_gui.h"

#define i2c_Address 0x3c

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SENSOR_PIN_LEFT 12
#define SENSOR_PIN_MIDDLE 27
#define SENSOR_PIN_RIGHT 14

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

HAL hal = HAL(&display, {SENSOR_PIN_LEFT, SENSOR_PIN_MIDDLE, SENSOR_PIN_RIGHT});

void setup() {
  Serial.begin(9600);
  counter_gui::setup(&hal);

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
