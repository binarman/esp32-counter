#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <numeric>
#include <initializer_list>
#include <string.h>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define SENSOR_PIN_LEFT 12
#define SENSOR_PIN_MIDDLE 12
#define SENSOR_PIN_RIGHT 12

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
}

class ThreeStateButtonWidget: public Widget {
  int off_x;
  int off_y;
  const char *short_press_text;
  const char *long_press_text;
  int control_pin_no;
  ButtonState<2> state;
public:
  ThreeStateButtonWidget() : off_x(0), off_y(0), short_press_text(nullptr), long_press_text(nullptr), control_pin_no(-1), state({100, 1500}) {
  }

  void initialize(int offset_x, int offset_y, const char *short_press_label, const char *long_press_label, int control_pin) {
    off_x = offset_x;
    off_y = offset_y;
    short_press_text = short_press_label;
    long_press_text = long_press_label;
    control_pin_no = control_pin;
    pinMode(control_pin_no, INPUT_PULLUP);
    state.reset();
  }

  void reset() override{
    state.reset();
  }

  void update() override{
    bool pin_state = digitalRead(control_pin_no);
    int timestamp = esp_timer_get_time() / 1000;
    int event = state.updateState(timestamp, !pin_state);
    // todo callback on release
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
public:
  TwoStateButtonWidget() : off_x(0), off_y(0), press_text(nullptr), control_pin_no(-1), state({100}) {
  }

  void initialize(int offset_x, int offset_y, const char *press_label, int control_pin) {
    off_x = offset_x;
    off_y = offset_y;
    press_text = press_label;
    control_pin_no = control_pin;
    pinMode(control_pin_no, INPUT_PULLUP);
    state.reset();
  }

  void reset() override{
    state.reset();
  }

  void update() override{
    bool pin_state = digitalRead(control_pin_no);
    int timestamp = esp_timer_get_time() / 1000;
    int event = state.updateState(timestamp, !pin_state);
    // todo callback on release
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

class CounterWidget {
  int state;
  int addition;
public:
  void reset() override {
    // Do nothing?
  }
  void update() override {
    
  }
  void draw() override {
    
  }
}

ThreeStateButtonWidget plus_minus_1;
ThreeStateButtonWidget plus_minus_5;
TwoStateButtonWidget menu;

void setup() {
  plus_minus_1.initialize(0, SCREEN_HEIGHT-11, "+1", "-1", SENSOR_PIN_LEFT);
  plus_minus_5.initialize(SCREEN_WIDTH/2 - 15, SCREEN_HEIGHT-11, "+5", "-5", SENSOR_PIN_MIDDLE);
  menu.initialize(SCREEN_WIDTH - 4*6, SCREEN_HEIGHT-11, "menu", SENSOR_PIN_RIGHT);
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
  plus_minus_1.update();
  plus_minus_5.update();
  menu.update();

  // draw interface  
  display.clearDisplay();
  plus_minus_1.draw();
  plus_minus_5.draw();
  menu.draw();
  display.display();
}
