#include "counter_gui.h"
#include "widgets.h"
#include <cstdio>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LEFT_BUTTON_ID 0
#define MIDDLE_BUTTON_ID 1
#define RIGHT_BUTTON_ID 2

namespace {
ThreeStateButtonWidget plus_minus_1;
ThreeStateButtonWidget plus_minus_5;
ThreeStateButtonWidget commit_reject;
TwoStateButtonWidget menu;
CounterWidget counter;
ListWidget<8> short_history;

enum class VisibleScreen {
  Starting,
  Addition,
  Menu
};

VisibleScreen screen;

void adjust1Release(int event) {
  if (event == 1 || event == 2) {
    counter.changeDelta(event == 1 ? 1 : -1);
    screen = VisibleScreen::Addition;
  }
}

void adjust5Release(int event) {
  if (event == 1 || event == 2) {
    counter.changeDelta(event == 1 ? 5 : -5);
    screen = VisibleScreen::Addition;
  }
}

void commitRejectRelease(int event) {
  static int items_counter = 1;
  if (event > 0)
    screen = VisibleScreen::Starting;
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
  screen = VisibleScreen::Starting;
  // initialize lower panel
  plus_minus_1.setParams("+1", "-1", LEFT_BUTTON_ID, adjust1Release);
  plus_minus_5.setParams("+5", "-5", MIDDLE_BUTTON_ID, adjust5Release);
  menu.setParams("menu", RIGHT_BUTTON_ID, commitRejectRelease);
  commit_reject.setParams("ok", "drop", RIGHT_BUTTON_ID, commitRejectRelease);

  int lower_panel_y = std::max(std::max(std::max(plus_minus_1.getH(), plus_minus_5.getH()), menu.getH()), commit_reject.getH());
  plus_minus_1.setPos(hal, 0, lower_panel_y);
  plus_minus_5.setPos(hal, SCREEN_WIDTH/2 - plus_minus_5.getW(), lower_panel_y);
  menu.setPos(hal, SCREEN_WIDTH - menu.getW(), lower_panel_y);
  commit_reject.setPos(hal, SCREEN_WIDTH - commit_reject.getW(), lower_panel_y);
  //initialize main screen
  counter.setPos(hal, 0, 0);
  short_history.setPos(hal, 72, 0);
  counter.setParams(72, lower_panel_y); // todo fix dimensions of counter
  short_history.setParams(SCREEN_WIDTH-counter.getW(), SCREEN_HEIGHT - lower_panel_y);
}

void loop() {
  // update state
  if (screen == VisibleScreen::Starting) {
    plus_minus_1.update();
    plus_minus_5.update();
    menu.update();
    counter.update();
    short_history.update();
  }
  if (screen == VisibleScreen::Addition) {
    plus_minus_1.update();
    plus_minus_5.update();
    commit_reject.update();
    counter.update();
  }
  // draw interface
  if (screen == VisibleScreen::Starting) {
    plus_minus_1.draw();
    plus_minus_5.draw();
    menu.draw();
    counter.draw();
    short_history.draw();
  }
  if (screen == VisibleScreen::Addition) {
    plus_minus_1.draw();
    plus_minus_5.draw();
    commit_reject.draw();
    counter.draw();
  }
}

} // namespace counter_gui
