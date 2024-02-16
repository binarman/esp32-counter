#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <numeric>
#include <initializer_list>
#include <vector>

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

#define SENSOR_PIN 12
#define NUM_SAMPLES 10

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

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
class ButtonState {
  int state;
  int last_press_time;
  int last_update_time;
  std::vector<int> milestones;
public:
  ButtonState(std::initializer_list<int> milestones) : state(-1), last_press_time(0), last_update_time(0), milestones(milestones) {}

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
    while (state < milestones.size() && elapsed >= milestones[state])
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
    float maxTime = milestones.back();
    float progress = (last_update_time - last_press_time) / maxTime;
    return std::min(progress, 1.0f);
  }
};

class TwoStateButtonWidget {
  int off_x;
  int off_y;
  const char *short_press_text;
  const char *long_press_text;
  int control_pin_no;
  ButtonState state;
public:
  TwoStateButtonWidget() : off_x(0), off_y(0), short_press_text(nullptr), long_press_text(nullptr), control_pin_no(-1), state({300, 2000}) {
  }

  void reset(int offset_x, int offset_y, const char *short_press_label, const char *long_press_label, int control_pin) {
    off_x = offset_x;
    off_y = offset_y;
    short_press_text = short_press_label;
    long_press_text = long_press_label;
    control_pin_no = control_pin;
    pinMode(control_pin_no, INPUT_PULLUP);
    state.reset();
  }

  void update() {
    bool pin_state = digitalRead(control_pin_no);
    int timestamp = esp_timer_get_time() / 1000;
    int event = state.updateState(timestamp, !pin_state);
    // todo callback on release
  }

  void draw() {
    // draw labels
    int s = state.getState();
    display.setTextColor(SH110X_WHITE);
    display.setCursor(off_x, off_y);
    if (s == 1)
      display.setTextSize(2);
    else 
      display.setTextSize(1);
    display.println(short_press_text);
    if (s == 2)
      display.setTextSize(2);
    else 
      display.setTextSize(1);
    display.println(long_press_text);
    display.setTextSize(1);
    display.print("progress");
    display.println(state.getTimeFromPress());
    // draw progress bar
  }
};

TwoStateButtonWidget test_button;

void setup()   {
  test_button.reset(20, 20, "test 1", "test 2", SENSOR_PIN);
  Serial.begin(9600);

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
 //display.setContrast (0); // dim display
  //pinMode(SENSOR_PIN, OUTPUT);
 
  display.display();
  delay(2000);
}

void loop() {
//  static int samples[NUM_SAMPLES] = {};
//  static int cur_sample = 0;
//  int raw_value = analogRead(SENSOR_PIN);
//  samples[cur_sample] = raw_value;
//  cur_sample = (cur_sample + 1) % NUM_SAMPLES;
//
//  auto avg_value = std::accumulate(std::begin(samples), std::end(samples), 0) / NUM_SAMPLES;
//  float voltage = 3.3 / (2 << 11) * avg_value;

  // Clear the buffer.
  test_button.update();
  display.clearDisplay();

  test_button.draw();
  bool pin_state = digitalRead(SENSOR_PIN);
  if (pin_state)
    display.print("pull up");
  else
    display.print("pull down");
//  display.setTextSize(1);
//  display.setTextColor(SH110X_WHITE);
//  display.setCursor(0, 0);
//  display.print("Current voltage: ");
/////  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
//  display.println(voltage);
//  display.print("raw value: ");
//  display.println(raw_value);
////  display.setTextSize(2);
////  display.setTextColor(SH110X_WHITE);
////  display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.display();
}

//  // draw a single pixel
//  display.drawPixel(10, 10, SH110X_WHITE);
//  // Show the display buffer on the hardware.
//  // NOTE: You _must_ call display after making any drawing commands
//  // to make them visible on the display hardware!
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  // draw many lines
//  testdrawline();
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  // draw rectangles
//  testdrawrect();
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  // draw multiple rectangles
//  testfillrect();
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  // draw mulitple circles
//  testdrawcircle();
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  // draw a SH110X_WHITE circle, 10 pixel radius
//  display.fillCircle(display.width() / 2, display.height() / 2, 10, SH110X_WHITE);
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  testdrawroundrect();
//  delay(2000);
//  display.clearDisplay();
//
//  testfillroundrect();
//  delay(2000);
//  display.clearDisplay();
//
//  testdrawtriangle();
//  delay(2000);
//  display.clearDisplay();
//
//  testfilltriangle();
//  delay(2000);
//  display.clearDisplay();
//
//  // draw the first ~12 characters in the font
//  testdrawchar();
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//
//
//  // text display tests
//  display.setTextSize(1);
//  display.setTextColor(SH110X_WHITE);
//  display.setCursor(0, 0);
//  display.println("Failure is always an option");
//  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
//  display.println(3.141592);
//  display.setTextSize(2);
//  display.setTextColor(SH110X_WHITE);
//  display.print("0x"); display.println(0xDEADBEEF, HEX);
//  display.display();
//  delay(2000);
//  display.clearDisplay();
//
//  // miniature bitmap display
//  display.drawBitmap(30, 16,  logo16_glcd_bmp, 16, 16, 1);
//  display.display();
//  delay(1);
//
//  // invert the display
//  display.invertDisplay(true);
//  delay(1000);
//  display.invertDisplay(false);
//  delay(1000);
//  display.clearDisplay();
//
//  // draw a bitmap icon and 'animate' movement
//  testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_HEIGHT, LOGO16_GLCD_WIDTH);
//}
//
//
//
//
//void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
//  uint8_t icons[NUMFLAKES][3];
//
//  // initialize
//  for (uint8_t f = 0; f < NUMFLAKES; f++) {
//    icons[f][XPOS] = random(display.width());
//    icons[f][YPOS] = 0;
//    icons[f][DELTAY] = random(5) + 1;
//
//    Serial.print("x: ");
//    Serial.print(icons[f][XPOS], DEC);
//    Serial.print(" y: ");
//    Serial.print(icons[f][YPOS], DEC);
//    Serial.print(" dy: ");
//    Serial.println(icons[f][DELTAY], DEC);
//  }
//
//  while (1) {
//    // draw each icon
//    for (uint8_t f = 0; f < NUMFLAKES; f++) {
//      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SH110X_WHITE);
//    }
//    display.display();
//    delay(200);
//
//    // then erase it + move it
//    for (uint8_t f = 0; f < NUMFLAKES; f++) {
//      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SH110X_BLACK);
//      // move it
//      icons[f][YPOS] += icons[f][DELTAY];
//      // if its gone, reinit
//      if (icons[f][YPOS] > display.height()) {
//        icons[f][XPOS] = random(display.width());
//        icons[f][YPOS] = 0;
//        icons[f][DELTAY] = random(5) + 1;
//      }
//    }


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
  delay(1);
}

void testdrawcircle(void) {
  for (int16_t i = 0; i < display.height(); i += 2) {
    display.drawCircle(display.width() / 2, display.height() / 2, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i = 0; i < display.height() / 2; i += 3) {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.display();
    delay(1);
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5) {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, SH110X_WHITE);
    display.display();
    delay(1);
  }
}

void testfilltriangle(void) {
  uint8_t color = SH110X_WHITE;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5) {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, SH110X_WHITE);
    if (color == SH110X_WHITE) color = SH110X_BLACK;
    else color = SH110X_WHITE;
    display.display();
    delay(1);
  }
}

void testdrawroundrect(void) {
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, SH110X_WHITE);
    display.display();
    delay(1);
  }
}

void testfillroundrect(void) {
  uint8_t color = SH110X_WHITE;
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, color);
    if (color == SH110X_WHITE) color = SH110X_BLACK;
    else color = SH110X_WHITE;
    display.display();
    delay(1);
  }
}

void testdrawrect(void) {
  for (int16_t i = 0; i < display.height() / 2; i += 2) {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, SH110X_WHITE);
    display.display();
    delay(1);
  }
}

void testdrawline() {
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, 0, i, display.height() - 1, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(0, 0, display.width() - 1, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, display.height() - 1, i, 0, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = display.width() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.height(); i += 4) {
    display.drawLine(display.width() - 1, 0, 0, i, SH110X_WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i = 0; i < display.width(); i += 4) {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, SH110X_WHITE);
    display.display();
    delay(1);
  }
  delay(250);
}
