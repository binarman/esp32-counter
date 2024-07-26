#ifndef SCREENS_H
#define SCREENS_H

#include "widgets.h"
#include <cstdio>

#define LEFT_BUTTON_ID 0
#define MIDDLE_BUTTON_ID 1
#define RIGHT_BUTTON_ID 2

namespace counter_gui {

constexpr int lower_panel_height = 11;
constexpr int max_counter_font_size = 6;
constexpr int counter_width = 2 * max_counter_font_size * CHAR_W;

class Screen {
  Widget *w[MAX_WIDGETS];
  int size = 0;
  int screen_width = 0;
  int screen_height = 0;
  int lower_panel_y = 0;

public:
  void setup(HAL *h) {
    size = 0;
    screen_width = h->display()->width();
    screen_height = h->display()->height();
    lower_panel_y = screen_height - lower_panel_height;
  }

  int width() const { return screen_width; }

  int height() const { return screen_height; }

  int panelY() const { return lower_panel_y; }

  void addWidget(Widget *widget) { w[size++] = widget; }

  bool update() {
    bool updated = false;
    for (int i = 0; i < size; ++i)
      updated |= w[i]->update();
    return updated;
  }

  void draw() {
    for (int i = 0; i < size; ++i)
      w[i]->draw();
  }
};

class MainScreen : public Screen {
  LabelWidget<> counter;
  OverwritingListWidget<8> short_history;
  ThreeStateButtonWidget plus_minus_1;
  ThreeStateButtonWidget plus_minus_5;
  TwoStateButtonWidget menu;

  int counter_value = 0;

public:
  void setup(HAL *hal, std::function<void(int)> oneRelease,
             std::function<void(int)> fiveRelease,
             std::function<void(int)> menuRelease) {
    Screen::setup(hal);

    plus_minus_1.setParams("+1", "-1", LEFT_BUTTON_ID, oneRelease);
    plus_minus_5.setParams("+5", "-5", MIDDLE_BUTTON_ID, fiveRelease);
    menu.setParams("menu", RIGHT_BUTTON_ID, menuRelease);

    plus_minus_1.setPos(hal, 0, panelY());
    plus_minus_5.setPos(hal, (width() - plus_minus_5.getW()) / 2, panelY());
    menu.setPos(hal, width() - menu.getW(), panelY());

    // initialize main screen widgets
    counter.setPos(hal, 0, 0);
    short_history.setPos(hal, counter_width, 0);

    counter.setParams(72, panelY(), HAlign::LEFT);
    short_history.setParams(width() - counter.getW(), panelY());

    setCounter(0);

    addWidget(&plus_minus_1);
    addWidget(&plus_minus_5);
    addWidget(&menu);
    addWidget(&counter);
    addWidget(&short_history);
  }

  int getCounter() { return counter_value; }

  void setCounter(int c) {
    counter_value = c;
    counter.setFormattedLabel("%d", c);
  }

  void reset_history() { short_history.reset(); }

  void addHistoryItem(const char *item) { short_history.addItem(item); }
};

class DeltaScreen : public Screen {
  LabelWidget<> counter;
  LabelWidget<> delta;
  LabelWidget<> new_counter;
  ThreeStateButtonWidget plus_minus_1;
  ThreeStateButtonWidget plus_minus_5;
  ThreeStateButtonWidget commit_reject;

  int counter_value = 0;
  int delta_value = 0;

  void setDelta(int d) {
    delta_value = d;
    if (d >= 0)
      delta.setFormattedLabel("+%d", delta_value);
    else
      delta.setFormattedLabel("%d", delta_value);
    new_counter.setFormattedLabel("=%d", counter_value + delta_value);
  }

  void adjust1Release(int event) {
    int diff = event == 1 ? 1 : -1;
    setDelta(delta_value + diff);
  }

  void adjust5Release(int event) {
    int diff = event == 1 ? 5 : -5;
    setDelta(delta_value + diff);
  }

public:
  void setup(HAL *hal, std::function<void(int)> commitRejectRelease) {
    Screen::setup(hal);
    plus_minus_1.setParams("+1", "-1", LEFT_BUTTON_ID,
                           [this](int event) { adjust1Release(event); });
    plus_minus_5.setParams("+5", "-5", MIDDLE_BUTTON_ID,
                           [this](int event) { adjust5Release(event); });
    commit_reject.setParams("ok", "drop", RIGHT_BUTTON_ID, commitRejectRelease);

    plus_minus_1.setPos(hal, 0, panelY());
    plus_minus_5.setPos(hal, (width() - plus_minus_5.getW()) / 2, panelY());
    commit_reject.setPos(hal, width() - commit_reject.getW(), panelY());

    // initialize main screen widgets
    counter.setPos(hal, 0, 0);
    delta.setPos(hal, counter_width, 0);
    new_counter.setPos(hal, counter_width, CHAR_H);

    counter.setParams(72, panelY(), HAlign::LEFT);
    delta.setParams(width() - counter_width, CHAR_H, HAlign::LEFT);
    new_counter.setParams(width() - counter_width, CHAR_H, HAlign::LEFT);

    addWidget(&plus_minus_1);
    addWidget(&plus_minus_5);
    addWidget(&commit_reject);
    addWidget(&counter);
    addWidget(&delta);
    addWidget(&new_counter);
  }

  void setCounterAndDelta(int c, int d) {
    counter_value = c;
    delta_value = d;
    counter.setFormattedLabel("%d", c);
    setDelta(d);
  }

  int getDelta() { return delta_value; }
};

class MenuScreen : public Screen {
  ListWithSelectorWidget<5> menu_items;
  RepeatingButtonWidget menu_up;
  RepeatingButtonWidget menu_down;
  ThreeStateButtonWidget select_return;

  void menuUpRelease(int event) { menu_items.moveSelUp(); }

  void menuDownRelease(int event) { menu_items.moveSelDown(); }

public:
  void setup(HAL *hal, std::function<void(int)> selectReturnRelease) {
    Screen::setup(hal);
    menu_items.setParams(width(), height() - lower_panel_height, 0);
    menu_items.addItem("show full history");
    menu_items.addItem("start new counting");
    menu_items.addItem("drop full history");
    menu_up.setParams("\x1e", LEFT_BUTTON_ID,
                      [this](int event) { menuUpRelease(event); });
    menu_down.setParams("\x1f", MIDDLE_BUTTON_ID,
                        [this](int event) { menuDownRelease(event); });
    select_return.setParams("sel", "back", RIGHT_BUTTON_ID,
                            selectReturnRelease);

    menu_items.setPos(hal, 0, 0);
    menu_up.setPos(hal, 0, panelY());
    menu_down.setPos(hal, (width() - menu_down.getW()) / 2, panelY());
    select_return.setPos(hal, width() - select_return.getW(), panelY());

    addWidget(&menu_items);
    addWidget(&menu_up);
    addWidget(&menu_down);
    addWidget(&select_return);
  }

  int getSelPos() { return menu_items.getSelPos(); }
};

class HistoryScreen : public Screen {
  RepeatingButtonWidget history_up;
  RepeatingButtonWidget history_down;
  TwoStateButtonWidget history_return;
  OverwritingListWidget<128> history_items;

  void historyUpRelease(int event) { history_items.moveUp(); }

  void historyDownRelease(int event) { history_items.moveDown(); }

public:
  void setup(HAL *hal, std::function<void(int)> returnRelease) {
    Screen::setup(hal);

    history_items.setParams(width(), height() - lower_panel_height);
    history_up.setParams("\x1e", LEFT_BUTTON_ID,
                         [this](int event) { historyUpRelease(event); });
    history_down.setParams("\x1f", MIDDLE_BUTTON_ID,
                           [this](int event) { historyDownRelease(event); });
    history_return.setParams("back", RIGHT_BUTTON_ID, returnRelease);

    history_items.setPos(hal, 0, 0);
    history_up.setPos(hal, 0, panelY());
    history_down.setPos(hal, (width() - history_down.getW()) / 2, panelY());
    history_return.setPos(hal, width() - history_return.getW(), panelY());

    addWidget(&history_items);
    addWidget(&history_up);
    addWidget(&history_down);
    addWidget(&history_return);
  }

  void addHistoryItem(const char *item) { history_items.addItem(item); }

  void clearHistory() { history_items.reset(); }
};

class AcceptScreen : public Screen {
  const char *m = nullptr;
  LabelWidget<> common_prompt;
  LabelWidget<> detailed_prompt;
  TwoStateButtonWidget ok;
  TwoStateButtonWidget cancel;

public:
  void setup(HAL *hal, const char *message, std::function<void(int)> ok_action,
             std::function<void(int)> cancel_action) {
    Screen::setup(hal);
    common_prompt.setParams(width(), CHAR_H, HAlign::MIDDLE, "confirm to");
    detailed_prompt.setParams(width(), CHAR_H, HAlign::MIDDLE, message);
    ok.setParams("ok", 0, ok_action);
    cancel.setParams("cancel", 2, cancel_action);

    common_prompt.setPos(hal, 0, CHAR_H);
    detailed_prompt.setPos(hal, 0, CHAR_H * 3);
    ok.setPos(hal, 0, panelY());
    cancel.setPos(hal, width() - cancel.getW(), panelY());

    addWidget(&common_prompt);
    addWidget(&detailed_prompt);
    addWidget(&ok);
    addWidget(&cancel);
  }
};

} // namespace counter_gui

#endif // SCREENS_H
