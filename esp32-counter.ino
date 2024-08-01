#include "counter_gui.h"
#include <esp_sleep.h>

#define i2c_Address 0x3c

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define LEFT_BTN_PIN 12
#define MID_BTN_PIN 27
#define RIGHT_BTN_PIN 14
#define POWER_PIN 34

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_FRAM_I2C raw_mem;
PersistentMemoryWrapper mem(&raw_mem, 1 << 15);
HAL hal(&display, &mem, {LEFT_BTN_PIN, MID_BTN_PIN, RIGHT_BTN_PIN}, POWER_PIN);

void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(9600);
  mem.setup();
  counter_gui::setup(&hal);

  delay(250);
  display.begin(i2c_Address, true);

  display.display();
  delay(1000);
}

void loop() {
  bool updated = counter_gui::update();
  if (updated) {
    display.clearDisplay();
    counter_gui::draw();
    display.display();
  } else {
    constexpr int timeout_us = 20000;
    esp_err_t timer_set = esp_sleep_enable_timer_wakeup(timeout_us);
    if (timer_set == ESP_OK)
      esp_light_sleep_start();
  }
}
