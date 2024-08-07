#ifndef HAL_H
#define HAL_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>

constexpr float average_battery_voltage = 1.4;
constexpr float upper_battery_voltage = 1.7;
// Assuming 1 or 2 ordinary chemical batteries are used
// Each battery gives voltage somewhere between 1.2 and 1.5 Volts
// All computations are very approximate
class BatteryState {
  int adc_bit_width;
  float vcc_voltage;

public:
  BatteryState() {}

  BatteryState(int adc_bit_width, float vcc_voltage) {
    init(adc_bit_width, vcc_voltage);
  }

  void init(int adc_bit_width, float vcc_voltage) {
    this->adc_bit_width = adc_bit_width;
    this->vcc_voltage = vcc_voltage;
    assert(adc_bit_width > 0 && adc_bit_width < 32);
    assert(vcc_voltage > 0 && "VCC voltage should be positive");
  }

  float convertProbeValToState(int battery_probe_value) const {
    assert(battery_probe_value >= 0 &&
           battery_probe_value < (1 << adc_bit_width));
    float probe_voltage = static_cast<float>(battery_probe_value) /
                          (1 << adc_bit_width) * vcc_voltage;
    int num_batteries = std::ceil(probe_voltage / upper_battery_voltage);
    assert(num_batteries >= 0 && num_batteries <= 3 &&
           "unexpected number of batteries detected");
    if (num_batteries == 0)
      return -1.0;
    float expected_full_voltage = num_batteries * average_battery_voltage;
    return std::min(probe_voltage / expected_full_voltage, 1.0f);
  }
};

#ifdef TEST_MODE
#include <memory>

class PersistentMemory {
  std::unique_ptr<uint8_t[]> data;
  bool valid;

public:
  PersistentMemory(bool valid, int size)
      : data(new uint8_t[size]), valid(valid) {
    std::fill(data.get(), data.get() + size, 0);
  }

  bool begin() const { return valid; }

  uint8_t read(int addr) const { return data[addr]; }

  void write(int addr, uint8_t value) { data[addr] = value; }
};

#else // TEST_MODE
#include "Adafruit_FRAM_I2C.h"

using PersistentMemory = Adafruit_FRAM_I2C;

#endif // TEST_MODE

class PersistentMemoryWrapper {
private:
  PersistentMemory *raw_mem;
  bool valid;
  int mem_size;

public:
  PersistentMemoryWrapper(PersistentMemory *raw_mem, int size)
      : raw_mem(raw_mem), mem_size(size), valid(false) {}

  void setup() {
    if (raw_mem->begin())
      valid = true;
  }

  bool isValid() const { return valid; }

  int size() const { return mem_size; }

  uint8_t read(int addr) const { return raw_mem->read(addr); }

  void write(int addr, uint8_t value) { raw_mem->write(addr, value); }
};

#ifdef TEST_MODE

#include <gmock/gmock.h>

enum Color {
  BLACK = 0,
  WHITE = 1,
  INVERSE = 2,
};

class Display {
public:
  MOCK_METHOD(void, dim, (uint8_t contrast));
  MOCK_METHOD(void, drawPixel, (int16_t x, int16_t y, uint16_t color));
  MOCK_METHOD(void, drawFastVLine,
              (int16_t x, int16_t y, int16_t h, uint16_t color));
  MOCK_METHOD(void, drawFastHLine,
              (int16_t x, int16_t y, int16_t w, uint16_t color));
  MOCK_METHOD(void, setTextSize, (uint8_t s));
  MOCK_METHOD(void, setTextColor, (uint16_t c));
  MOCK_METHOD(void, setCursor, (int16_t x, int16_t y));
  MOCK_METHOD(size_t, print, (const char s[]));
  MOCK_METHOD(size_t, print, (char c));
  MOCK_METHOD(size_t, print, (int val));
  MOCK_METHOD(void, drawRect,
              (uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
               uint16_t color));
  MOCK_METHOD(void, fillRect,
              (uint16_t x0, uint16_t y0, uint16_t w, uint16_t h,
               uint16_t color));
  MOCK_METHOD(uint16_t, width, ());
  MOCK_METHOD(uint16_t, height, ());
};

class HAL {
  Display *d;
  PersistentMemoryWrapper *mem;

public:
  HAL(Display *d, PersistentMemoryWrapper *mem) : d(d), mem(mem) {}

  Display *display() const { return d; }

  PersistentMemoryWrapper *persistentMemory() const { return mem; }

  MOCK_METHOD(bool, buttonPressed, (int button_no), (const));
  MOCK_METHOD(unsigned long, uptimeMillis, (), (const));
  MOCK_METHOD(float, getPowerState, (), (const));
};

#else // TEST_MODE
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <algorithm>
#include <initializer_list>

enum Color {
  BLACK = SH110X_BLACK,
  WHITE = SH110X_WHITE,
  INVERSE = SH110X_INVERSE,
};

using Display = Adafruit_SH1106G;

constexpr int MAX_BUTTONS = 3;

class HAL {
  Display *d;
  PersistentMemoryWrapper *mem;
  int button_pin[MAX_BUTTONS];
  int power_probe_pin;
  BatteryState bs;

public:
  HAL(Display *d, PersistentMemoryWrapper *mem,
      std::initializer_list<int> button_pins, int power_probe_pin)
      : d(d), mem(mem), power_probe_pin(power_probe_pin), bs(12, 3.3f) {
    std::copy(button_pins.begin(), button_pins.end(), button_pin);
    std::for_each(button_pins.begin(), button_pins.end(),
                  [](int pin) { pinMode(pin, INPUT_PULLUP); });
  }

  Display *display() const { return d; }

  PersistentMemoryWrapper *persistentMemory() const { return mem; }

  bool buttonPressed(int button_no) const {
    return !digitalRead(button_pin[button_no]);
  }

  unsigned long uptimeMillis() const { return millis(); }

  float getPowerState() const {
    int battery_probe_value = analogRead(power_probe_pin);
    return bs.convertProbeValToState(battery_probe_value);
  }
};

#endif // TEST_MODE

#endif // HAL_H
