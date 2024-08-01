#include "counter_gui.h"
#include "screens.h"
#include "state.h"
#include "widgets.h"
#include <cstdio>

#define MAX_SCREEN_DEPTH 5

namespace counter_gui {

namespace {

PersistentState saved_state;

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

void changeCounter(int new_value, int delta) {
  int old_value = new_value - delta;
  // update short history
  short_history_counter++;
  char history_item[MAX_HIST_STR_LEN + 1];
  char sign = delta >= 0 ? '+' : '-';
  snprintf(history_item, MAX_HIST_STR_LEN, "%d.%c%d", short_history_counter,
           sign, std::abs(delta));
  main_screen.addHistoryItem(history_item);

  // update full history
  global_history_counter++;
  snprintf(history_item, MAX_HIST_STR_LEN, "%d. %d=%d%c%d",
           global_history_counter, new_value, old_value, sign, std::abs(delta));
  history_screen.addHistoryItem(history_item);

  main_screen.setCounter(new_value);
}

void startNewCounting() {
  history_screen.addHistoryItem("------");
  short_history_counter = 0;
  main_screen.setCounter(0);
  main_screen.reset_history();
}

void clearHistory() {
  startNewCounting();
  history_screen.clearHistory();
  global_history_counter = 0;
}

// callbacks moving between screens

// Handler for ok/drop button on delta screen
void onCommitRejectDelta(int event) {
  popScreen();
  if (event == 1) {
    const int counter_value = main_screen.getCounter();
    const int delta_value = delta_screen.getDelta();
    int new_counter_value = counter_value + delta_value;
    saved_state.rememberNewValue(new_counter_value);
    changeCounter(new_counter_value, delta_value);
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

// Handler for confirm button confirm_remove_history screen
void onDeleteHistoryConfirmation(int event) {
  saved_state.rememberClearHistory();
  clearHistory();
  popScreen();
}

// Handler for confirm button confirm_new_count screen
void onNewCountConfirmation(int event) {
  saved_state.rememberStartNewCount();
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

  saved_state.setup(hal->persistentMemory());
  saved_state.restoreFromMem(changeCounter, clearHistory, startNewCounting);
}

bool update() { return getActiveScreen()->update(); }

void draw() { getActiveScreen()->draw(); }

} // namespace counter_gui
