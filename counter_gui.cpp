#include "counter_gui.h"
#include "widgets.h"
#include <cstdio>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LEFT_BUTTON_ID 0
#define MIDDLE_BUTTON_ID 1
#define RIGHT_BUTTON_ID 2
#define MAX_SCREEN_DEPTH 5

namespace {
ThreeStateButtonWidget plus_minus_1;
ThreeStateButtonWidget plus_minus_5;
ThreeStateButtonWidget commit_reject;
TwoStateButtonWidget menu;
CounterWidget counter;
ListWidget<8> short_history;

Screen main_screen;
Screen delta_screen;
Screen menu_screen;

Screen *screen[MAX_SCREEN_DEPTH];
int active_screen;

void pushScreen(Screen *s) {
  screen[++active_screen] = s;
}

Screen *getActiveScreen() {
  return screen[active_screen];
}

void gotoScreen(Screen *s) {
  if (getActiveScreen() != s)
    pushScreen(s);
}

void popScreen() {
  active_screen--;
}

void adjust1Release(int event) {
  if (event == 1 || event == 2) {
    counter.changeDelta(event == 1 ? 1 : -1);
    gotoScreen(&delta_screen);
  }
}

void adjust5Release(int event) {
  if (event == 1 || event == 2) {
    counter.changeDelta(event == 1 ? 5 : -5);
    gotoScreen(&delta_screen);
  }
}

void commitRejectRelease(int event) {
  static int items_counter = 1;
  if (event > 0)
    popScreen();
  if (event == 1) {
    int delta = counter.getDelta();
    char history_item[MAX_HIST_STR_LEN+1];
    char sign = delta >= 0 ? '+' : '-';
    snprintf(history_item, MAX_HIST_STR_LEN, "%d.%c%d", items_counter, sign, std::abs(delta));
    items_counter++;
    short_history.addItem(history_item);
    short_history.moveDown();
    counter.commitDelta();
  }
  if (event == 2)
    counter.rejectDelta();
}

void menuRelease(int event) {
  // goto menu
}
}

namespace counter_gui {

void setup(HAL *hal) {
  active_screen = 0;
  screen[0] = &main_screen;
  // initialize lower panel
  plus_minus_1.setParams("+1", "-1", LEFT_BUTTON_ID, adjust1Release);
  plus_minus_5.setParams("+5", "-5", MIDDLE_BUTTON_ID, adjust5Release);
  menu.setParams("menu", RIGHT_BUTTON_ID, menuRelease);
  commit_reject.setParams("ok", "drop", RIGHT_BUTTON_ID, commitRejectRelease);

  int lower_panel_y = SCREEN_HEIGHT - std::max(std::max(std::max(plus_minus_1.getH(), plus_minus_5.getH()), menu.getH()), commit_reject.getH());
  plus_minus_1.setPos(hal, 0, lower_panel_y);
  plus_minus_5.setPos(hal, SCREEN_WIDTH/2 - plus_minus_5.getW(), lower_panel_y);
  menu.setPos(hal, SCREEN_WIDTH - menu.getW(), lower_panel_y);
  commit_reject.setPos(hal, SCREEN_WIDTH - commit_reject.getW(), lower_panel_y);
  //initialize main screen
  counter.setPos(hal, 0, 0);
  short_history.setPos(hal, 72, 0);
  counter.setParams(72, lower_panel_y); // todo fix dimensions of counter
  short_history.setParams(SCREEN_WIDTH-counter.getW(), lower_panel_y);

  // initialize screens
  main_screen.setup();
  delta_screen.setup();
  menu_screen.setup();

  main_screen.addWidget(&plus_minus_1);
  main_screen.addWidget(&plus_minus_5);
  main_screen.addWidget(&menu);
  main_screen.addWidget(&counter);
  main_screen.addWidget(&short_history);

  delta_screen.addWidget(&plus_minus_1);
  delta_screen.addWidget(&plus_minus_5);
  delta_screen.addWidget(&commit_reject);
  delta_screen.addWidget(&counter);
}

void loop() {
  getActiveScreen()->update();
  getActiveScreen()->draw();
}

} // namespace counter_gui
