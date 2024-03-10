#ifndef COUNTER_GUI_H
#define COUNTER_GUI_H

#include "hal.h"

namespace counter_gui {

void setup(HAL *hal);

bool update();

void draw();

} // namespace counter_gui

#endif // COUNTER_GUI_H
