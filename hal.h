#ifndef HAL_H
#define HAL_H

#ifdef TEST_MODE
#include <cstdint>
#include <cstdlib>

constexpr int SH110X_WHITE = 1;
constexpr int DEC = 10;

class Display {
 public:
  void dim(uint8_t contrast);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void setTextSize(uint8_t s);
  void setTextColor(uint16_t c);
  void setCursor(int16_t x, int16_t y);

  size_t print(const char[]);
  size_t print(char);
  size_t print(unsigned char, int = DEC);
  size_t print(int, int = DEC);
};

constexpr uint8_t INPUT_PULLUP = 0xa;

void pinMode(uint8_t pin, uint8_t mode);

void digitalWrite(uint8_t pin, uint8_t val);

int digitalRead(uint8_t pin);

unsigned long millis();

#else
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
using Display = Adafruit_SH1106G;
#endif

#endif // HAL_H
