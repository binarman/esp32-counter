#ifndef WIDGETS_H
#define WIDGETS_H

#include "hal.h"
#include <cassert>
#include <functional>
#include <initializer_list>
#include <numeric>
#include <string.h>

#define MAX_HIST_STR_LEN 21
#define CHAR_W 6
#define CHAR_H 8
#define MAX_LABEL_LEN 21
#define MAX_WIDGETS 10

/**
 * @brief abstract class controlling button state
 *
 * Button holds several "milestone" timestamps.
 * Each milestone switches button to new state.
 *
 * when button is not pressed, it has state -1
 * when button is pressed, but did not reach any milestone yet, it has state 0
 * each reached milestone increases state counter by 1
 */
template <int NUM_MILESTONES> class ButtonState {
  int state;
  int last_press_time;
  int last_update_time;
  int milestones[NUM_MILESTONES];

public:
  ButtonState(std::initializer_list<int> milestones_time)
      : state(-1), last_press_time(0), last_update_time(0) {
    assert(milestones_time.size() == NUM_MILESTONES);
    std::copy(milestones_time.begin(), milestones_time.end(), milestones);
  }

  void reset() {
    state = -1;
    last_press_time = 0;
    last_update_time = 0;
  }

  int updateState(long timestamp, bool pressed) {
    last_update_time = timestamp;
    if (!pressed) {
      int emit_state = state;
      state = -1;
      return emit_state;
    }
    if (state == -1) {
      last_press_time = timestamp;
      state = 0;
    }
    int elapsed = timestamp - last_press_time;
    while (state < NUM_MILESTONES && elapsed >= milestones[state])
      state++;
    return -1;
  }

  int getState() const { return state; }

  int getTimeFromPress() const {
    if (state == -1)
      return 0;
    return last_update_time - last_press_time;
  }

  float getProgress() const {
    if (state == -1)
      return 0.0f;
    float maxTime = milestones[NUM_MILESTONES - 1];
    float progress = (last_update_time - last_press_time) / maxTime;
    return std::min(progress, 1.0f);
  }
};

class Widget {
protected:
  HAL *hal = nullptr;
  Display *display;
  int off_x = 0;
  int off_y = 0;

public:
  void setPos(HAL *h, int offset_x, int offset_y) {
    hal = h;
    display = h->display();
    off_x = offset_x;
    off_y = offset_y;
  }

  virtual ~Widget() = default;
  int getX() const { return off_x; }
  int getY() const { return off_y; }
  virtual int getW() const = 0;
  virtual int getH() const = 0;
  virtual void reset() = 0;
  virtual bool update() = 0;
  virtual void draw() const = 0;
};

class ThreeStateButtonWidget : public Widget {
  const char *short_press_text = nullptr;
  const char *long_press_text = nullptr;
  int button_id = -1;
  ButtonState<2> state = {50, 1000};
  std::function<void(int)> on_release = nullptr;

public:
  void setParams(const char *short_press_label, const char *long_press_label,
                 int btn_id, const std::function<void(int)> &callback) {
    short_press_text = short_press_label;
    long_press_text = long_press_label;
    button_id = btn_id;
    state.reset();
    on_release = callback;
  }

  int getW() const override {
    int short_press_length = strlen(short_press_text);
    int long_press_length = strlen(long_press_text);
    return (short_press_length + long_press_length + 1) * CHAR_W;
  }

  int getH() const override { return CHAR_H + 3; }

  void reset() override { state.reset(); }

  bool update() override {
    bool button_state = hal->buttonPressed(button_id);
    auto timestamp = hal->uptimeMillis();
    auto old_progress = state.getProgress();
    int event = state.updateState(timestamp, button_state);
    bool state_changed =
        (state.getProgress() != old_progress || button_state || event > 0);
    if (event > 0)
      on_release(event);
    return state_changed;
  }

  void draw() const override {
    // draw labels
    int s = state.getState();
    display->setTextColor(Color::WHITE);
    int short_press_length = strlen(short_press_text) * CHAR_W;
    int long_press_length = strlen(long_press_text) * CHAR_W;
    int first_part_length = short_press_length + CHAR_W;

    char full_label[MAX_LABEL_LEN + 1] = {0};
    strncat(full_label, short_press_text, MAX_LABEL_LEN);
    strncat(full_label, "/", MAX_LABEL_LEN);
    strncat(full_label, long_press_text, MAX_LABEL_LEN);
    full_label[MAX_LABEL_LEN] = '\0';
    display->setCursor(off_x, off_y);
    display->setTextSize(1);
    display->print(full_label);

    if (s == 1)
      display->drawFastHLine(off_x, off_y + CHAR_H, short_press_length,
                             Color::WHITE);
    if (s == 2)
      display->drawFastHLine(off_x + first_part_length, off_y + CHAR_H,
                             long_press_length, Color::WHITE);
    // draw progress bar
    float progress = state.getProgress();
    if (progress > 0.0f) {
      int full_length = first_part_length + long_press_length;
      int progress_bar_len = full_length * progress;
      display->drawFastHLine(off_x, off_y + CHAR_H + 2, progress_bar_len,
                             Color::WHITE);
    }
  }
};

class TwoStateButtonWidget : public Widget {
  const char *press_text = nullptr;
  int button_id = -1;
  ButtonState<1> state = {50};
  std::function<void(int)> on_release = nullptr;

public:
  void setParams(const char *press_label, int btn_id,
                 const std::function<void(int)> callback) {
    press_text = press_label;
    button_id = btn_id;
    state.reset();
    on_release = callback;
  }

  int getW() const override {
    int press_text_length = strlen(press_text);
    return press_text_length * CHAR_W;
  }

  int getH() const override { return CHAR_H + 1; }

  void reset() override { state.reset(); }

  bool update() override {
    bool button_state = hal->buttonPressed(button_id);
    int timestamp = hal->uptimeMillis();
    auto old_progress = state.getProgress();
    int event = state.updateState(timestamp, button_state);
    bool state_changed =
        (state.getProgress() != old_progress || button_state || event > 0);
    if (event > 0)
      on_release(event);
    return state_changed;
  }

  void draw() const override {
    int s = state.getState();
    display->setTextColor(Color::WHITE);
    int press_text_length = strlen(press_text) * CHAR_W;
    display->setCursor(off_x, off_y);
    display->setTextSize(1);
    display->print(press_text);
    if (s == 1)
      display->drawFastHLine(off_x, off_y + 8, press_text_length, Color::WHITE);
  }
};

template <class Derived, int MAX_ITEM_LEN>
class ListWidgetBase : public Widget {
protected:
  int w = -1;
  int h = -1;
  int first_visible_item = 0;
  bool updated = true;

  Derived &d() { return *static_cast<Derived *>(this); }
  const Derived &d() const { return *static_cast<const Derived *>(this); }

public:
  void setParams(int width, int height) {
    w = width;
    h = height;
    first_visible_item = 0;
    updated = true;
  }

  void moveDown() {
    int visible_items = h / CHAR_H;
    if (first_visible_item < d().getSize() - visible_items) {
      first_visible_item++;
      updated = true;
    }
  }

  void moveUp() {
    if (first_visible_item > 0) {
      first_visible_item--;
      updated = true;
    }
  }

  int getW() const override { return w; }

  int getH() const override { return h; }

  void reset() override {
    first_visible_item = 0;
    updated = true;
  }

  void draw() const override {
    display->setTextColor(Color::WHITE);
    const int size = d().getSize();
    int num_items_to_print = std::min(h / CHAR_H, size - first_visible_item);
    int max_item_width = std::min(w / CHAR_W, MAX_ITEM_LEN);
    char print_buffer[MAX_ITEM_LEN + 1];
    display->setTextSize(1);
    for (int i = 0; i < num_items_to_print; ++i) {
      display->setCursor(off_x, off_y + i * CHAR_H);
      d().getItem(first_visible_item + i, print_buffer, MAX_ITEM_LEN);
      print_buffer[max_item_width] = '\0';
      display->print(print_buffer);
    }
  }

  bool update() override {
    if (updated) {
      updated = false;
      return true;
    }
    return false;
  }
};

template <int MAX_ITEMS, int MAX_ITEM_LEN = MAX_HIST_STR_LEN>
class OverwritingListWidget
    : public ListWidgetBase<OverwritingListWidget<MAX_ITEMS, MAX_ITEM_LEN>,
                            MAX_ITEM_LEN> {
  char items[MAX_ITEMS][MAX_ITEM_LEN + 1];
  int size = 0;
  int insert_point = 0; // items organized in rolling list, if storage
                        // overflows, overwrite old items
  using Base = ListWidgetBase<OverwritingListWidget<MAX_ITEMS, MAX_ITEM_LEN>,
                              MAX_ITEM_LEN>;

public:
  void setParams(int width, int height) {
    reset();
    Base::setParams(width, height);
  }

  void addItem(const char *item) {
    strncpy(items[insert_point], item, MAX_ITEM_LEN);
    items[insert_point][MAX_ITEM_LEN] = '\0';
    if (size < MAX_ITEMS)
      size++;
    insert_point = (insert_point + 1) % MAX_ITEMS;
    Base::updated = true;
  }

  int getSize() const { return size; }

  void getItem(int i, char *buffer, int max_str_len) const {
    int first_item_pos = (insert_point - size + MAX_ITEMS) % MAX_ITEMS;
    int item_pos = (first_item_pos + i) % MAX_ITEMS;
    strncpy(buffer, items[item_pos], max_str_len);
    buffer[max_str_len] = '\0';
  }

  void reset() override {
    Base::reset();
    size = 0;
    insert_point = 0;
  }
};

template <int MAX_ITEMS, int MAX_LEN = MAX_HIST_STR_LEN>
class ListWithSelectorWidget
    : public ListWidgetBase<ListWithSelectorWidget<MAX_ITEMS, MAX_LEN>,
                            MAX_LEN> {
  int sel_pos = 0;
  char items[MAX_ITEMS][MAX_LEN + 1];
  int size = 0;
  using Base =
      ListWidgetBase<ListWithSelectorWidget<MAX_ITEMS, MAX_LEN>, MAX_LEN>;

public:
  void setParams(int width, int height, int selector_pos) {
    Base::setParams(width, height);
    size = 0;
    sel_pos = selector_pos;
  }

  void addItem(const char *item) {
    strncpy(items[size], item, MAX_LEN);
    items[size][MAX_LEN] = '\0';
    assert(size < MAX_ITEMS);
    size++;
    Base::updated = true;
  }

  int getSize() const { return size; }

  void getItem(int i, char *buffer, int max_str_len) const {
    assert(max_str_len > 1);
    if (i == sel_pos)
      buffer[0] = '\x1a';
    else
      buffer[0] = ' ';
    buffer[1] = '\0';
    strncat(buffer, items[i], max_str_len);
    buffer[max_str_len] = '\0';
  }

  void reset() override {
    Base::reset();
    size = 0;
    sel_pos = 0;
  }

  void adjustVisibleAreaToSel() {
    if (sel_pos < Base::first_visible_item)
      Base::first_visible_item = sel_pos;
    int last_visible_item =
        Base::first_visible_item + this->getH() / CHAR_H - 1;
    if (sel_pos > last_visible_item)
      Base::first_visible_item += sel_pos - last_visible_item;
  }

  int getSelPos() const { return sel_pos; }

  void moveSelUp() {
    sel_pos = std::max(sel_pos - 1, 0);
    adjustVisibleAreaToSel();
    Base::updated = true;
  }

  void moveSelDown() {
    sel_pos = std::min(sel_pos + 1, size - 1);
    adjustVisibleAreaToSel();
    Base::updated = true;
  }
};

enum HAlign { LEFT, MIDDLE, RIGHT };

template <int MAX_LEN = MAX_HIST_STR_LEN> class LabelWidget : public Widget {
  int w = -1;
  int h = -1;
  char value[MAX_LEN + 1];
  HAlign a;
  bool updated = true;

public:
  template <typename... Args>
  void setFormattedLabel(const char *format, Args... args) {
    snprintf(value, MAX_LEN, format, args...);
    value[MAX_LEN] = '\0';
    updated = true;
  }

  void setParams(int width, int height, HAlign align,
                 const char *label = nullptr) {
    w = width;
    h = height;
    a = align;
    if (label != nullptr) {
      strncpy(value, label, MAX_LEN);
      value[MAX_LEN] = '\0';
    } else
      value[0] = '\0';
    updated = true;
  }

  int getW() const override { return w; }

  int getH() const override { return h; }

  void reset() override { updated = true; }

  bool update() override {
    if (updated) {
      updated = false;
      return true;
    }
    return false;
  }

  void draw() const override {
    const int str_len = strlen(value);
    if (str_len == 0)
      return;
    const int w_font_limit = w / (str_len * CHAR_W);
    const int h_font_limit = h / CHAR_H;
    const int font_size = std::min(w_font_limit, h_font_limit);
    assert(font_size > 0);
    int x_alignment = -1;
    // assume y alignment as "top"
    int y_alignment = 0;
    switch (a) {
    case HAlign::LEFT:
      x_alignment = 0;
      break;
    case HAlign::MIDDLE:
      x_alignment = (w - font_size * CHAR_W * str_len) / 2;
      break;
    case HAlign::RIGHT:
      x_alignment = w - font_size * CHAR_W * str_len;
      break;
    }
    display->setTextColor(Color::WHITE);
    display->setCursor(off_x + x_alignment, off_y + y_alignment);
    display->setTextSize(font_size);
    display->print(value);
  }
};

class BatteryWidget : public Widget {
  int state = -2;

public:
  void setParams() { reset(); }

  int getW() const override { return 16; }
  int getH() const override { return 7; }
  void reset() override { state = -2; }

  bool update() override {
    float raw_state = hal->getPowerState();
    int new_state = 0;
    if (raw_state < 0)
      new_state = -1;
    if (raw_state > 0.25)
      new_state++;
    if (raw_state > 0.5)
      new_state++;
    if (raw_state > 0.75)
      new_state++;
    if (new_state != state) {
      state = new_state;
      return true;
    }
    return false;
  }

  void draw() const override {
    if (state == -1) {
      display->drawFastHLine(off_x, off_y + 1, 3, Color::WHITE);
      display->drawFastHLine(off_x, off_y + 3, 3, Color::WHITE);
      display->drawFastHLine(off_x + 8, off_y + 2, 3, Color::WHITE);
      display->drawRect(off_x + 3, off_y, 3, 5, Color::WHITE);
      display->drawRect(off_x + 6, off_y + 1, 2, 3, Color::WHITE);
    } else {
      display->drawRect(off_x + 1, off_y, 15, 7, Color::WHITE);
      display->drawFastVLine(off_x, off_y + 1, 5, Color::WHITE);
      if (state > 2)
        display->fillRect(off_x + 3, off_y + 2, 3, 3, Color::WHITE);
      if (state > 1)
        display->fillRect(off_x + 7, off_y + 2, 3, 3, Color::WHITE);
      if (state > 0)
        display->fillRect(off_x + 11, off_y + 2, 3, 3, Color::WHITE);
    }
  }
};

#endif // WIDGETS_H
