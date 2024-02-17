#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <numeric>
#include <initializer_list>
#include <functional>
#include <string.h>

#define i2c_Address 0x3c

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SENSOR_PIN_LEFT 12
#define SENSOR_PIN_MIDDLE 27
#define SENSOR_PIN_RIGHT 14

#define MAX_HIST_STR_LEN 21

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
public:
  virtual void reset() = 0;
  virtual void update() = 0;
  virtual void draw() = 0;
};

class ThreeStateButtonWidget: public Widget {
  int off_x;
  int off_y;
  const char *short_press_text;
  const char *long_press_text;
  int control_pin_no;
  ButtonState<2> state;
  std::function<void(int)> on_release;
public:
  ThreeStateButtonWidget() : off_x(0), off_y(0), short_press_text(nullptr), long_press_text(nullptr), control_pin_no(-1), state({50, 1000}) {
  }

  void initialize(int offset_x, int offset_y, const char *short_press_label, const char *long_press_label, int control_pin, const std::function<void(int)> &callback) {
    off_x = offset_x;
    off_y = offset_y;
    short_press_text = short_press_label;
    long_press_text = long_press_label;
    control_pin_no = control_pin;
    pinMode(control_pin_no, INPUT_PULLUP);
    state.reset();
    on_release = callback;
  }

  void reset() override{
    state.reset();
  }

  void update() override{
    bool pin_state = digitalRead(control_pin_no);
    int timestamp = esp_timer_get_time() / 1000;
    int event = state.updateState(timestamp, !pin_state);
    on_release(event);
  }

  void draw() override{
    // draw labels
    int s = state.getState();
    constexpr int second_part_offset = 0;
    display.setTextColor(SH110X_WHITE);
    int short_press_length = strlen(short_press_text) * 6;
    int long_press_length = strlen(long_press_text) * 6;
    int first_part_length = short_press_length + 6;
    display.setCursor(off_x, off_y);
    display.setTextSize(1);
    display.print(short_press_text);
    display.print("/");
    display.setCursor(off_x + first_part_length, off_y + second_part_offset);
    display.print(long_press_text);

    if (s == 1)
      display.drawFastHLine(off_x, off_y + 8, short_press_length, SH110X_WHITE);
    if (s == 2)
      display.drawFastHLine(off_x + first_part_length, off_y + second_part_offset + 8, long_press_length, SH110X_WHITE);
    // draw progress bar
    float progress = state.getProgress();
    if (progress > 0.0f) {
      int full_length = first_part_length + long_press_length;
      int progress_bar_len = full_length * progress;
      display.drawFastHLine(off_x, off_y + second_part_offset + 10, progress_bar_len, SH110X_WHITE);
    }
  }
};

class TwoStateButtonWidget: public Widget {
  int off_x;
  int off_y;
  const char *press_text;
  int control_pin_no;
  ButtonState<1> state;
  std::function<void(int)> on_release;
public:
  TwoStateButtonWidget() : off_x(0), off_y(0), press_text(nullptr), control_pin_no(-1), state({100}) {
  }

  void initialize(int offset_x, int offset_y, const char *press_label, int control_pin, const std::function<void(int)> callback) {
    off_x = offset_x;
    off_y = offset_y;
    press_text = press_label;
    control_pin_no = control_pin;
    pinMode(control_pin_no, INPUT_PULLUP);
    state.reset();
    on_release = callback;
  }

  void reset() override{
    state.reset();
  }

  void update() override{
    bool pin_state = digitalRead(control_pin_no);
    int timestamp = esp_timer_get_time() / 1000;
    int event = state.updateState(timestamp, !pin_state);
    on_release(event);
  }

  void draw() override{
    int s = state.getState();
    display.setTextColor(SH110X_WHITE);
    int press_text_length = strlen(press_text) * 6;
    display.setCursor(off_x, off_y);
    display.setTextSize(1);
    display.print(press_text);
    if (s == 1)
      display.drawFastHLine(off_x, off_y + 8, press_text_length, SH110X_WHITE);
  }
};

template<int MAX_ITEMS, int MAX_LEN = MAX_HIST_STR_LEN>
class ListWidget: public Widget {
  int off_x;
  int off_y;
  int w;
  int h;
  char items[MAX_ITEMS][MAX_LEN+1];
  int size;
  int first_visible_item;
public:
  ListWidget() : off_x(0), off_y(0), w(0), h(0), size(0), first_visible_item(0) {}

  void initialize(int offset_x, int offset_y, int width, int height) {
    off_x = offset_x;
    off_y = offset_y;
    w = width;
    h = height;
    size = 0;
    first_visible_item = 0;
  }

  void addItem(const char *item) {
    if (size == MAX_ITEMS)
      size = 0;
    strncpy(items[size], item, MAX_LEN);
    items[size][MAX_LEN] = '\0';
    size++;
  }

  void moveDown() {
    if (first_visible_item < size-1)
      first_visible_item++;
  }

  void moveUp() {
    if (first_visible_item > 0)
      first_visible_item--;
  }

  void reset() override {
  }

  void update() override {
  }

  void draw() override {
    display.setTextColor(SH110X_WHITE);
    int num_items_to_print = std::min(h / 8, size - first_visible_item);
    int max_item_width = std::min(w / 6, MAX_LEN);
    char print_buffer[MAX_LEN+2+1];
    display.setTextSize(1);
    for (int i = 0; i < num_items_to_print; ++i) {
      display.setCursor(off_x, off_y + i*8);
      int item_no = i + first_visible_item;
      strncpy(print_buffer, items[item_no], MAX_LEN);
      print_buffer[max_item_width] = '\0';
      display.print(print_buffer);
    }
  }
};

class CounterWidget: public Widget {
public:
  enum State {
    ShowHistory,
    ShowDelta
  };
private:
  State state;
  int counter;
  int delta;
  int off_x;
  int off_y;
public:

  CounterWidget(): state(State::ShowHistory), counter(0), delta(0), off_x(0), off_y(0) {}

  void initialize(int offset_x, int offset_y) {
    off_x = offset_x;
    off_y = offset_y;
  }

  int getDelta() {
    return delta;
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

  void reset() override {
  }

  void update() override {
  }

  void draw() override {
    constexpr int font_size = 5;
    constexpr int max_counter_digits = 3;
    constexpr int max_counter_len = font_size * max_counter_digits * 6;
    display.setTextColor(SH110X_WHITE);
    display.setCursor(off_x, off_y);
    display.setTextSize(5);
    display.print(counter);
    if (state == State::ShowDelta) {
      display.setTextSize(1);
      display.setCursor(off_x + max_counter_len, off_y);
      if (delta >= 0)
        display.print("+");
      display.print(delta);
      display.setCursor(off_x + max_counter_len, off_y + 8);
      display.print("=");
      display.print(counter + delta);
    }
  }
};

ThreeStateButtonWidget plus_minus_1;
ThreeStateButtonWidget plus_minus_5;
ThreeStateButtonWidget commit_reject;
TwoStateButtonWidget menu;
CounterWidget counter;
ListWidget<10> short_history;

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
    snprintf(history_item, MAX_HIST_STR_LEN, "%d. %c%d", items_counter, sign, std::abs(delta));
    short_history.addItem(history_item);
    counter.commitDelta();
  }
  if (event == 2)
    counter.rejectDelta();
}

void menuRelease(int event) {
  // goto menu
}

void setup() {
  screen = VisibleScreen::Starting;
  plus_minus_1.initialize(0, SCREEN_HEIGHT-11, "+1", "-1", SENSOR_PIN_LEFT, adjust1Release);
  plus_minus_5.initialize(SCREEN_WIDTH/2 - 15, SCREEN_HEIGHT-11, "+5", "-5", SENSOR_PIN_MIDDLE, adjust5Release);
  menu.initialize(SCREEN_WIDTH - 4*6, SCREEN_HEIGHT-11, "menu", SENSOR_PIN_RIGHT, commitRejectRelease);
  commit_reject.initialize(SCREEN_WIDTH - 7*6, SCREEN_HEIGHT-11, "ok", "drop", SENSOR_PIN_RIGHT, commitRejectRelease);
  counter.initialize(0, 0);
  short_history.initialize(72, 0, 128-72, 64-11);
  Serial.begin(9600);

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  delay(250);
  display.begin(i2c_Address, true);
 
  display.display();
  delay(2000);
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
  display.clearDisplay();

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
  display.display();
}
