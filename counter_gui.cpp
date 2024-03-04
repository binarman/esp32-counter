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
OverwritingListWidget<8> short_history;

ListWithSelectorWidget<5> menu_items;
TwoStateButtonWidget menu_up;
TwoStateButtonWidget menu_down;
ThreeStateButtonWidget select_return;

TwoStateButtonWidget history_up;
TwoStateButtonWidget history_down;
TwoStateButtonWidget history_return;
OverwritingListWidget<128> history_items;

Screen main_screen;
Screen delta_screen;
Screen menu_screen;
Screen history_screen;
AcceptScreen confirm_remove_history_screen;
AcceptScreen confirm_new_count_screen;

// counter state
int items_counter = 1;
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
  if (event > 0)
    popScreen();
  if (event == 1) {
    // update short history
    int delta = counter.getDelta();
    char history_item[MAX_HIST_STR_LEN+1];
    char sign = delta >= 0 ? '+' : '-';
    snprintf(history_item, MAX_HIST_STR_LEN, "%d.%c%d", items_counter, sign, std::abs(delta));
    short_history.addItem(history_item);
    short_history.moveDown();

    // update full history
    int value = counter.getValue();
    int new_value = value + delta;
    snprintf(history_item, MAX_HIST_STR_LEN, "%d. %d=%d%c%d", items_counter, new_value, value, sign, std::abs(delta));
    history_items.addItem(history_item);

    items_counter++;
    counter.commitDelta();
  }
  if (event == 2)
    counter.rejectDelta();
}

void menuRelease(int event) {
  if (event > 0)
    gotoScreen(&menu_screen);
}

void menuUpRelease(int event) {
  if (event > 0)
    menu_items.moveSelUp();
}

void menuDownRelease(int event) {
  if (event > 0)
    menu_items.moveSelDown();
}

void selectReturnRelease(int event) {
  if (event == 1) {
    switch (menu_items.getSelPos()){
      case 0:
        gotoScreen(&history_screen);
        break;
      case 1:
        gotoScreen(&confirm_new_count_screen);
        break;
      case 2:
        gotoScreen(&confirm_remove_history_screen);
        break;
    }
  }
  if (event == 2) {
    popScreen();
  }
}

void returnRelease(int event) {
  if (event > 0)
    popScreen();
}

void historyUpRelease(int event) {
  if (event > 0)
    history_items.moveUp();
}

void historyDownRelease(int event) {
  if (event > 0)
    history_items.moveDown();
}

void deleteHistoryRelease(int event) {
  if (event > 0) {
    history_items.reset();
    counter.reset();
    short_history.reset();
    popScreen();
  }
}

void newCountRelease(int event) {
  if (event > 0) {
    history_items.addItem("------");
    counter.reset();
    short_history.reset();
    popScreen();
  }
}

}

namespace counter_gui {

void setup(HAL *hal) {
  active_screen = 0;
  items_counter = 1;
  screen[0] = &main_screen;
  const int lower_panel_height = 11;
  // initialize main screen and delta widgets
  plus_minus_1.setParams("+1", "-1", LEFT_BUTTON_ID, adjust1Release);
  plus_minus_5.setParams("+5", "-5", MIDDLE_BUTTON_ID, adjust5Release);
  menu.setParams("menu", RIGHT_BUTTON_ID, menuRelease);
  commit_reject.setParams("ok", "drop", RIGHT_BUTTON_ID, commitRejectRelease);

  int lower_panel_y = SCREEN_HEIGHT - lower_panel_height;
  plus_minus_1.setPos(hal, 0, lower_panel_y);
  plus_minus_5.setPos(hal, (SCREEN_WIDTH - plus_minus_5.getW())/2, lower_panel_y);
  menu.setPos(hal, SCREEN_WIDTH - menu.getW(), lower_panel_y);
  commit_reject.setPos(hal, SCREEN_WIDTH - commit_reject.getW(), lower_panel_y);

  // initialize main screen widgets
  counter.setPos(hal, 0, 0);
  short_history.setPos(hal, 72, 0);
  counter.setParams(72, lower_panel_y); // todo fix dimensions of counter
  short_history.setParams(SCREEN_WIDTH-counter.getW(), lower_panel_y);

  // initialize menu widgets
  menu_items.setParams(SCREEN_WIDTH, SCREEN_HEIGHT - lower_panel_height, 0);
  menu_items.addItem("show full history");
  menu_items.addItem("start new counting");
  menu_items.addItem("drop full history");
  menu_up.setParams("\x1e", LEFT_BUTTON_ID, menuUpRelease);
  menu_down.setParams("\x1f", MIDDLE_BUTTON_ID, menuDownRelease);
  select_return.setParams("sel", "back", RIGHT_BUTTON_ID, selectReturnRelease);

  menu_items.setPos(hal, 0, 0);
  menu_up.setPos(hal, 0, lower_panel_y);
  menu_down.setPos(hal, (SCREEN_WIDTH - menu_down.getW())/2, lower_panel_y);
  select_return.setPos(hal, SCREEN_WIDTH - select_return.getW(), lower_panel_y);

  // initialize history widgets
  history_items.setParams(SCREEN_WIDTH, SCREEN_HEIGHT - lower_panel_height);
  history_up.setParams("\x1e", LEFT_BUTTON_ID, historyUpRelease);
  history_down.setParams("\x1f", MIDDLE_BUTTON_ID, historyDownRelease);
  history_return.setParams("back", RIGHT_BUTTON_ID, returnRelease);

  history_items.setPos(hal, 0, 0);
  history_up.setPos(hal, 0, lower_panel_y);
  history_down.setPos(hal, (SCREEN_WIDTH - history_down.getW())/2, lower_panel_y);
  history_return.setPos(hal, SCREEN_WIDTH - history_return.getW(), lower_panel_y);


  // initialize screens
  main_screen.setup();
  delta_screen.setup();
  menu_screen.setup();
  history_screen.setup();
  confirm_remove_history_screen.setup(hal, SCREEN_WIDTH, SCREEN_HEIGHT, "delete history", deleteHistoryRelease, returnRelease);
  confirm_new_count_screen.setup(hal, SCREEN_WIDTH, SCREEN_HEIGHT, "start new count", newCountRelease, returnRelease);

  main_screen.addWidget(&plus_minus_1);
  main_screen.addWidget(&plus_minus_5);
  main_screen.addWidget(&menu);
  main_screen.addWidget(&counter);
  main_screen.addWidget(&short_history);

  delta_screen.addWidget(&plus_minus_1);
  delta_screen.addWidget(&plus_minus_5);
  delta_screen.addWidget(&commit_reject);
  delta_screen.addWidget(&counter);

  menu_screen.addWidget(&menu_items);
  menu_screen.addWidget(&menu_up);
  menu_screen.addWidget(&menu_down);
  menu_screen.addWidget(&select_return);

  history_screen.addWidget(&history_items);
  history_screen.addWidget(&history_up);
  history_screen.addWidget(&history_down);
  history_screen.addWidget(&history_return);
}

void loop() {
  getActiveScreen()->update();
  getActiveScreen()->draw();
}

} // namespace counter_gui
