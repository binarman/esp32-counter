#ifndef HAL_H
#define HAL_H

#include <cstdint>
#include <cstdlib>

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

public:
  HAL(Display *d) : d(d) {}

  Display *display() const { return d; }

  MOCK_METHOD(bool, buttonPressed, (int button_no), (const));
  MOCK_METHOD(unsigned long, uptimeMillis, (), (const));
  MOCK_METHOD(float, getPowerState, (), (const));
};

#else
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
  int button_pin[MAX_BUTTONS];

public:
  HAL(Display *d, std::initializer_list<int> button_pins) : d(d) {
    std::copy(button_pins.begin(), button_pins.end(), button_pin);
    std::for_each(button_pins.begin(), button_pins.end(),
                  [](int pin) { pinMode(pin, INPUT_PULLUP); });
  }

  Display *display() const { return d; }

  bool buttonPressed const(int button_no) {
    return !digitalRead(button_pin[button_no]);
  }

  unsigned long uptimeMillis() const { return millis(); }

  float getPowerState() const { return (millis() % 5000) / 5000.0; }
};

#endif

#endif // HAL_H
