#include "counter_gui.h"
#include "screens.h"
#include "widgets.h"
#include <cstdio>

#define MAX_SCREEN_DEPTH 5

namespace counter_gui {

namespace {

BatteryWidget battery;

MainScreen main_screen;
DeltaScreen delta_screen;
MenuScreen menu_screen;
HistoryScreen history_screen;
AcceptScreen confirm_remove_history_screen;
AcceptScreen confirm_new_count_screen;

// global state and screen state
int global_history_counter = 0;
int short_history_counter = 0;
Screen *screen[MAX_SCREEN_DEPTH];
int active_screen;

void pushScreen(Screen *s) { screen[++active_screen] = s; }
Screen *getActiveScreen() { return screen[active_screen]; }
void popScreen() { active_screen--; }

// callbacks moving between screens

// Handler for ok/drop button on delta screen
void onCommitRejectDelta(int event) {
  popScreen();
  if (event == 1) {
    // update short history
    short_history_counter++;
    char history_item[MAX_HIST_STR_LEN + 1];
    const int delta_value = delta_screen.getDelta();
    char sign = delta_value >= 0 ? '+' : '-';
    snprintf(history_item, MAX_HIST_STR_LEN, "%d.%c%d", short_history_counter,
             sign, std::abs(delta_value));
    main_screen.addHistoryItem(history_item);

    // update full history
    global_history_counter++;
    const int counter_value = main_screen.getCounter();
    int new_counter_value = counter_value + delta_value;
    snprintf(history_item, MAX_HIST_STR_LEN, "%d. %d=%d%c%d",
             global_history_counter, new_counter_value, counter_value, sign,
             std::abs(delta_value));
    history_screen.addHistoryItem(history_item);

    main_screen.setCounter(new_counter_value);
  }
}

// Handler for +1/-1 button on main screen
void onSmallAdjustMainScreen(int event) {
  int diff = (event == 1 ? 1 : -1);
  pushScreen(&delta_screen);
  delta_screen.setCounterAndDelta(main_screen.getCounter(), diff);
}

// Handler for +5/-5 button on main screen
void onLargeAdjustMainScreen(int event) {
  int diff = (event == 1 ? 5 : -5);
  pushScreen(&delta_screen);
  delta_screen.setCounterAndDelta(main_screen.getCounter(), diff);
}

// Handler for menu button on main screen
void onMenuPress(int event) { pushScreen(&menu_screen); }

// Handler for item select on menu screen
void onItemSelect(int event) {
  if (event == 1) {
    switch (menu_screen.getSelPos()) {
    case 0:
      popScreen();
      break;
    case 1:
      pushScreen(&history_screen);
      break;
    case 2:
      pushScreen(&confirm_new_count_screen);
      break;
    case 3:
      pushScreen(&confirm_remove_history_screen);
      break;
    }
  }
}

// Handler for back button on history screen and confirmation screens
void onReturn(int event) { popScreen(); }

void startNewCounting() {
  short_history_counter = 0;
  main_screen.setCounter(0);
  main_screen.reset_history();
}

// Handler for confirm button confirm_remove_history screen
void onDeleteHistoryConfirmation(int event) {
  history_screen.clearHistory();
  global_history_counter = 0;
  startNewCounting();
  popScreen();
}

// Handler for confirm button confirm_new_count screen
void onNewCountConfirmation(int event) {
  history_screen.addHistoryItem("------");
  startNewCounting();
  popScreen();
}

} // namespace

void setup(HAL *hal) {
  active_screen = 0;
  short_history_counter = 0;
  global_history_counter = 0;
  screen[0] = &main_screen;

  // initialize battery widget
  battery.setParams();
  battery.setPos(hal, 128 - battery.getW(), 0);

  // initialize screens
  main_screen.setup(hal, onSmallAdjustMainScreen, onLargeAdjustMainScreen,
                    onMenuPress);
  delta_screen.setup(hal, onCommitRejectDelta);
  menu_screen.setup(hal, onItemSelect);
  history_screen.setup(hal, onReturn);
  confirm_remove_history_screen.setup(hal, "delete history",
                                      onDeleteHistoryConfirmation, onReturn);
  confirm_new_count_screen.setup(hal, "start new count", onNewCountConfirmation,
                                 onReturn);

  main_screen.addWidget(&battery);
  delta_screen.addWidget(&battery);
  menu_screen.addWidget(&battery);
  history_screen.addWidget(&battery);
  confirm_remove_history_screen.addWidget(&battery);
  confirm_new_count_screen.addWidget(&battery);
}

bool update() { return getActiveScreen()->update(); }

void draw() { getActiveScreen()->draw(); }

} // namespace counter_gui
