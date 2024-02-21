#ifndef HAL_H
#define HAL_H

#ifdef TEST_MODE

class Display {
  // TBD
};

#else
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
using Display = Adafruit_SH1106G;
#endif

#endif // HAL_H
