#ifdef TEST_MODE

#include "hal.h"

void Display::dim(uint8_t contrast){}
void Display::drawPixel(int16_t x, int16_t y, uint16_t color){}
void Display::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){}
void Display::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){}
void Display::setTextSize(uint8_t s){}
void Display::setTextColor(uint16_t c){}
void Display::setCursor(int16_t x, int16_t y){}
size_t Display::print(const char v[]){}
size_t Display::print(char v){}
size_t Display::print(unsigned char v, int base){}
size_t Display::print(int v, int base){}

void pinMode(uint8_t pin, uint8_t mode){}

void digitalWrite(uint8_t pin, uint8_t val){}

int digitalRead(uint8_t pin){}

unsigned long millis(){}

#endif
