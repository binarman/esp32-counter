#ifndef WIDGETS_H
#define WIDGETS_H

#include "hal.h"
#include <numeric>
#include <initializer_list>
#include <functional>
#include <string.h>
#include <cassert>

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
template <int NUM_MILESTONES>
class ButtonState {
  int state;
  int last_press_time;
  int last_update_time;
  int milestones[NUM_MILESTONES];
public:
  ButtonState(std::initializer_list<int> milestones_time) : state(-1), last_press_time(0), last_update_time(0) {
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
    if (!pressed){
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

  int getState() {
    return state;
  }

  int getTimeFromPress() {
    if (state == -1)
      return 0;
    return last_update_time - last_press_time;
  }

  float getProgress() {
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
  int getX() { return off_x; }
  int getY() { return off_y; }
  virtual int getW() = 0;
  virtual int getH() = 0;
  virtual void reset() = 0;
  virtual void update() = 0;
  virtual void draw() = 0;
};

class ThreeStateButtonWidget: public Widget {
  const char *short_press_text = nullptr;
  const char *long_press_text = nullptr;
  int button_id = -1;
  ButtonState<2> state = {50, 1000};
  std::function<void(int)> on_release = nullptr;
public:
  void setParams(const char *short_press_label, const char *long_press_label, int btn_id, const std::function<void(int)> &callback) {
    short_press_text = short_press_label;
    long_press_text = long_press_label;
    button_id = btn_id;
    state.reset();
    on_release = callback;
  }

  int getW() override {
    int short_press_length = strlen(short_press_text);
    int long_press_length = strlen(long_press_text);
    return (short_press_length + long_press_length + 1) * CHAR_W;
  }

  int getH() override {
    return CHAR_H+3;
  }

  void reset() override{
    state.reset();
  }

  void update() override{
    bool button_state = hal->buttonPressed(button_id);
    auto timestamp = hal->uptimeMillis();
    int event = state.updateState(timestamp, button_state);
    on_release(event);
  }

  void draw() override {
    // draw labels
    int s = state.getState();
    display->setTextColor(SH110X_WHITE);
    int short_press_length = strlen(short_press_text) * CHAR_W;
    int long_press_length = strlen(long_press_text) * CHAR_W;
    int first_part_length = short_press_length + CHAR_W;

    char full_label[MAX_LABEL_LEN+1] = {0};
    strncat(full_label, short_press_text, MAX_LABEL_LEN);
    strncat(full_label, "/", MAX_LABEL_LEN);
    strncat(full_label, long_press_text, MAX_LABEL_LEN);
    full_label[MAX_LABEL_LEN] = '\0';
    display->setCursor(off_x, off_y);
    display->setTextSize(1);
    display->print(full_label);

    if (s == 1)
      display->drawFastHLine(off_x, off_y + CHAR_H, short_press_length, SH110X_WHITE);
    if (s == 2)
      display->drawFastHLine(off_x + first_part_length, off_y + CHAR_H, long_press_length, SH110X_WHITE);
    // draw progress bar
    float progress = state.getProgress();
    if (progress > 0.0f) {
      int full_length = first_part_length + long_press_length;
      int progress_bar_len = full_length * progress;
      display->drawFastHLine(off_x, off_y + CHAR_H + 2, progress_bar_len, SH110X_WHITE);
    }
  }
};

class TwoStateButtonWidget: public Widget {
  const char *press_text = nullptr;
  int button_id = -1;
  ButtonState<1> state = {50};
  std::function<void(int)> on_release = nullptr;
public:
  void setParams(const char *press_label, int btn_id, const std::function<void(int)> callback) {
    press_text = press_label;
    button_id = btn_id;
    state.reset();
    on_release = callback;
  }

  int getW() override {
    int press_text_length = strlen(press_text);
    return press_text_length * CHAR_W;
  }

  int getH() override {
    return CHAR_H + 1;
  }

  void reset() override{
    state.reset();
  }

  void update() override{
    bool button_state = hal->buttonPressed(button_id);
    int timestamp = hal->uptimeMillis();
    int event = state.updateState(timestamp, button_state);
    on_release(event);
  }

  void draw() override{
    int s = state.getState();
    display->setTextColor(SH110X_WHITE);
    int press_text_length = strlen(press_text) * CHAR_W;
    display->setCursor(off_x, off_y);
    display->setTextSize(1);
    display->print(press_text);
    if (s == 1)
      display->drawFastHLine(off_x, off_y + 8, press_text_length, SH110X_WHITE);
  }
};

template<class Derived, int MAX_ITEM_LEN>
class ListWidgetBase: public Widget {
protected:
  int w = -1;
  int h = -1;
  int first_visible_item = 0;

  Derived &d() {
    return *static_cast<Derived *>(this);
  }
public:
  void setParams(int width, int height) {
    w = width;
    h = height;
    first_visible_item = 0;
  }

  void moveDown() {
    int visible_items = h / CHAR_H;
    if (first_visible_item < d().getSize() - visible_items)
      first_visible_item++;
  }

  void moveUp() {
    if (first_visible_item > 0)
      first_visible_item--;
  }

  int getW() override {
    return w;
  }

  int getH() override {
    return h;
  }

  void draw() override {
    display->setTextColor(SH110X_WHITE);
    const int size = d().getSize();
    int num_items_to_print = std::min(h / CHAR_H, size - first_visible_item);
    int max_item_width = std::min(w / CHAR_W, MAX_ITEM_LEN);
    char print_buffer[MAX_ITEM_LEN+1];
    display->setTextSize(1);
    for (int i = 0; i < num_items_to_print; ++i) {
      display->setCursor(off_x, off_y + i * CHAR_H);
      d().getItem(first_visible_item + i, print_buffer, MAX_ITEM_LEN);
      print_buffer[max_item_width] = '\0';
      display->print(print_buffer);
    }
  }
};

template<int MAX_ITEMS, int MAX_ITEM_LEN = MAX_HIST_STR_LEN>
class OverwritingListWidget: public ListWidgetBase<OverwritingListWidget<MAX_ITEMS, MAX_ITEM_LEN>, MAX_ITEM_LEN> {
  char items[MAX_ITEMS][MAX_ITEM_LEN+1];
  int size = 0;
  int insert_point = 0; // items organized in rolling list, if storage overflows, overwrite old items
  using Base = ListWidgetBase<OverwritingListWidget<MAX_ITEMS, MAX_ITEM_LEN>, MAX_ITEM_LEN>;
public:
  void setParams(int width, int height) {
    Base::setParams(width, height);
    size = 0;
    insert_point = 0;
  }

  void addItem(const char *item) {
    strncpy(items[insert_point], item, MAX_ITEM_LEN);
    items[insert_point][MAX_ITEM_LEN] = '\0';
    if (size < MAX_ITEMS)
      size++;
    insert_point = (insert_point + 1) % MAX_ITEMS;
  }

  int getSize() {
    return size;
  }

  void getItem(int i, char *buffer, int max_str_len) {
    int first_item_pos = (insert_point - size + MAX_ITEMS) % MAX_ITEMS;
    int item_pos = (first_item_pos + i) % MAX_ITEMS;
    strncpy(buffer, items[item_pos], max_str_len);
    buffer[max_str_len] = '\0';
  }

  void reset() override {
  }

  void update() override {
  }
};

template<int MAX_ITEMS, int MAX_LEN = MAX_HIST_STR_LEN>
class ListWithSelectorWidget: public ListWidgetBase<ListWithSelectorWidget<MAX_ITEMS, MAX_LEN>, MAX_LEN> {
  int sel_pos = 0;
  char items[MAX_ITEMS][MAX_LEN+1];
  int size = 0;
  using Base = ListWidgetBase<ListWithSelectorWidget<MAX_ITEMS, MAX_LEN>, MAX_LEN>;
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
  }

  int getSize() {
    return size;
  }

  void getItem(int i, char *buffer, int max_str_len) {
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
  }

  void update() override {
  }

  void adjustVisibleAreaToSel() {
    if (sel_pos < Base::first_visible_item)
      Base::first_visible_item = sel_pos;
    int last_visible_item = Base::first_visible_item + this->getH() / CHAR_H - 1;
    if (sel_pos > last_visible_item)
      Base::first_visible_item += sel_pos - last_visible_item;
  }

  int getSelPos() {
    return sel_pos;
  }

  void moveSelUp() {
    sel_pos = std::max(sel_pos - 1, 0);
    adjustVisibleAreaToSel();
  }

  void moveSelDown() {
    sel_pos = std::min(sel_pos + 1, size - 1);
    adjustVisibleAreaToSel();
   }
};

class CounterWidget: public Widget {
public:
  enum State {
    ShowHistory,
    ShowDelta
  };
private:
  State state = State::ShowHistory;
  int counter = 0;
  int delta = 0;
  int w = -1;
  int h = -1;
public:
  void setParams(int width, int height) {
    state = State::ShowHistory;
    delta = 0;
    counter = 0;
    w = width;
    h = height;
  }

  int getDelta() {
    return delta;
  }

  int getValue() {
    return counter;
  }

  void changeDelta(int change) {
    delta += change;
    state = State::ShowDelta;
  }

  void commitDelta() {
    counter += delta;
    delta = 0;
    state = State::ShowHistory;
  }

  void rejectDelta() {
    delta = 0;
    state = State::ShowHistory;
  }

  int getW() override {
    return w;
  }

  int getH() override {
    return h;
  }

  void reset() override {
  }

  void update() override {
  }

  void draw() override {
    constexpr int font_size = 5;
    constexpr int max_counter_digits = 3;
    constexpr int max_counter_len = font_size * max_counter_digits * CHAR_W;
    display->setTextColor(SH110X_WHITE);
    display->setCursor(off_x, off_y);
    display->setTextSize(5);
    display->print(counter);
    if (state == State::ShowDelta) {
      display->setTextSize(1);
      display->setCursor(off_x + max_counter_len, off_y);
      if (delta >= 0)
        display->print("+");
      display->print(delta);
      display->setCursor(off_x + max_counter_len, off_y + CHAR_H);
      display->print("=");
      display->print(counter + delta);
    }
  }
};

class Screen {
  Widget *w[MAX_WIDGETS];
  int size = 0;
public:
  void setup() {
    size = 0;
  }

  void addWidget(Widget *widget){
    w[size++] = widget;
  }

  void update() {
    for (int i = 0; i < size; ++i)
      w[i]->update();
  }

  void draw() {
    for (int i = 0; i < size; ++i)
      w[i]->draw();
  }
};

class AcceptScreen: public Screen {
  const char *m = nullptr;
public:
  AcceptScreen(int screen_width, int screen_height){
    // TODO add message label, buttton 1, button 2
  }
  void initialize(const char *message) {
    m = message;
    // TODO update label widget
  }
};

#endif // WIDGETS_H
