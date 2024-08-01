#ifdef TEST_MODE

#include "counter_gui.h"
#include "screens.h"
#include "state.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::An;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::TypedEq;

#define CHAR_W 6
#define CHAR_H 8

void expectSetup(HAL &h) {
  auto &d = *h.display();
  ON_CALL(d, width()).WillByDefault(Return(128));
  ON_CALL(d, height()).WillByDefault(Return(64));
  EXPECT_CALL(d, width()).Times(6);
  EXPECT_CALL(d, height()).Times(6);
}

void expectUpdateButtons(HAL &h, int timestamp, bool btn1, bool btn2,
                         bool btn3) {
  EXPECT_CALL(h, uptimeMillis()).WillRepeatedly(Return(timestamp));
  EXPECT_CALL(h, buttonPressed(0)).WillRepeatedly(Return(btn1));
  EXPECT_CALL(h, buttonPressed(1)).WillRepeatedly(Return(btn2));
  EXPECT_CALL(h, buttonPressed(2)).WillRepeatedly(Return(btn3));
}

void expectBatteryState(HAL &h, float state) {
  EXPECT_CALL(h, getPowerState()).WillRepeatedly(Return(state));
}

void expectBatteryDraw(Display &d, float state = -1) {
  const int off_x = 112;
  const int off_y = 0;
  EXPECT_CALL(d, drawRect(off_x + 1, off_y, 15, 7, Color::WHITE))
      .WillRepeatedly(Return());
  EXPECT_CALL(d, drawFastVLine(off_x, off_y + 1, 5, Color::WHITE))
      .WillRepeatedly(Return());
  if (state > 0.75 || state < 0)
    EXPECT_CALL(d, fillRect(off_x + 3, off_y + 2, 3, 3, Color::WHITE))
        .WillRepeatedly(Return());
  if (state > 0.5 || state < 0)
    EXPECT_CALL(d, fillRect(off_x + 7, off_y + 2, 3, 3, Color::WHITE))
        .WillRepeatedly(Return());
  if (state > 0.25 || state < 0)
    EXPECT_CALL(d, fillRect(off_x + 11, off_y + 2, 3, 3, Color::WHITE))
        .WillRepeatedly(Return());
}

void expectMainScreen(Display &d, int counter) {
  EXPECT_CALL(d, setTextColor(1)).Times(5);
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setTextSize(1)).Times(4);
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+1/-1"))));
  EXPECT_CALL(d, setCursor(49, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+5/-5"))));
  EXPECT_CALL(d, setCursor(104, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("menu"))));
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setTextSize(6));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq(std::to_string(counter)))));
}

void expectMainScreenHistory(Display &d,
                             const std::vector<std::string> &entries) {
  for (int i = 0; i < entries.size(); ++i) {
    EXPECT_CALL(d, setCursor(72, i * 8));
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(entries[i]))));
  }
}

void expectDeltaScreen(Display &d, int counter, int delta) {
  EXPECT_CALL(d, setTextColor(1)).Times(6);
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setTextSize(1)).Times(5);

  EXPECT_CALL(d, setCursor(72, 0));
  std::string delta_str = (delta >= 0 ? "+" : "") + std::to_string(delta);
  std::string sum_str = "=" + std::to_string(counter + delta);
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq(delta_str))));
  EXPECT_CALL(d, setCursor(72, 8));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq(sum_str))));

  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+1/-1"))));
  EXPECT_CALL(d, setCursor(49, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+5/-5"))));
  EXPECT_CALL(d, setCursor(86, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("ok/drop"))));
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setTextSize(6));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq(std::to_string(counter)))));
}

void expectMenuScreen(Display &d, int selected_item) {
  EXPECT_CALL(d, setTextColor(1)).Times(4);
  EXPECT_CALL(d, setTextSize(1)).Times(4);
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setCursor(0, CHAR_H));
  EXPECT_CALL(d, setCursor(0, CHAR_H * 2));
  EXPECT_CALL(d, setCursor(0, CHAR_H * 3));
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setCursor(61, 53));
  EXPECT_CALL(d, setCursor(92, 53));
  if (selected_item == 0)
    EXPECT_CALL(d,
                print(Matcher<const char *>(StrEq("\x1ago to main screen"))));
  else
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(" go to main screen"))));
  if (selected_item == 1)
    EXPECT_CALL(d,
                print(Matcher<const char *>(StrEq("\x1ashow full history"))));
  else
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(" show full history"))));
  if (selected_item == 2)
    EXPECT_CALL(d,
                print(Matcher<const char *>(StrEq("\x1astart new counting"))));
  else
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(" start new counting"))));
  if (selected_item == 3)
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1a"
                                                     "drop full history"))));
  else
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(" drop full history"))));

  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1e"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1f"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("select"))));
}

void expectHistoryScreen(Display &d, const std::vector<std::string> &entries) {
  EXPECT_CALL(d, setTextColor(1)).Times(4);
  EXPECT_CALL(d, setTextSize(1)).Times(4);
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setCursor(61, 53));
  EXPECT_CALL(d, setCursor(104, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1e"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1f"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("back"))));
  for (int i = 0; i < entries.size(); ++i) {
    EXPECT_CALL(d, setCursor(0, i * 8));
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(entries[i]))));
  }
}

void expectMainScreenButtonAnimation(Display &d, float btn1, float btn2,
                                     float btn3) {
  if (btn1 >= 0)
    EXPECT_CALL(d,
                drawFastHLine(0, 63, (int)(btn1 * 5 * CHAR_W), Color::WHITE));
  if (btn1 == 1.0)
    EXPECT_CALL(d, drawFastHLine(3 * 6, 61, 2 * CHAR_W, Color::WHITE));
  else if (btn1 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(0, 61, 2 * CHAR_W, Color::WHITE));

  if (btn2 >= 0)
    assert(false && "todo implement");

  if (btn3 > 0)
    EXPECT_CALL(d, drawFastHLine(104, 61, 4 * CHAR_W, Color::WHITE));
}

void expectMenuScreenButtonAnimation(Display &d, bool btn1, bool btn2,
                                     bool btn3) {
  if (btn1)
    assert(false && "todo implement");

  if (btn2)
    assert(false && "todo implement");

  if (btn3)
    EXPECT_CALL(d, drawFastHLine(92, 61, (int)(6 * CHAR_W), Color::WHITE));
}

void expectDeltaScreenButtonAnimation(Display &d, float btn1, float btn2,
                                      float btn3) {
  if (btn1 >= 0)
    EXPECT_CALL(d,
                drawFastHLine(0, 63, (int)(btn1 * 5 * CHAR_W), Color::WHITE));
  if (btn1 == 1.0)
    EXPECT_CALL(d, drawFastHLine(3 * CHAR_W, 61, 2 * CHAR_W, Color::WHITE));
  else if (btn1 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(0, 61, 2 * CHAR_W, Color::WHITE));

  if (btn2 >= 0)
    EXPECT_CALL(d,
                drawFastHLine(49, 63, (int)(btn2 * 5 * CHAR_W), Color::WHITE));
  if (btn2 == 1.0)
    EXPECT_CALL(d,
                drawFastHLine(49 + 3 * CHAR_W, 61, 2 * CHAR_W, Color::WHITE));
  else if (btn2 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(49, 61, 2 * CHAR_W, Color::WHITE));

  if (btn3 >= 0)
    EXPECT_CALL(d,
                drawFastHLine(86, 63, (int)(btn3 * 7 * CHAR_W), Color::WHITE));
  if (btn3 == 1.0)
    assert(false && "todo implement");
  else if (btn3 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(86, 61, 2 * CHAR_W, Color::WHITE));
}

void loop() {
  counter_gui::update();
  counter_gui::draw();
}

void pressAndReleaseButtonsIgnoreOutput(HAL &h, bool btn1, bool btn2, bool btn3,
                                        int iteration_time, int iterations,
                                        int &timestamp) {
  auto &d = *h.display();
  EXPECT_CALL(d, setTextColor(::testing::_)).WillRepeatedly(Return());
  EXPECT_CALL(d, setTextSize(::testing::_)).WillRepeatedly(Return());
  EXPECT_CALL(d, setCursor(::testing::_, testing::_)).WillRepeatedly(Return());
  EXPECT_CALL(d, print(Matcher<const char *>(::testing::_)))
      .WillRepeatedly(Return(0));
  EXPECT_CALL(d, print(Matcher<int>(::testing::_))).WillRepeatedly(Return(0));
  EXPECT_CALL(
      d, drawFastHLine(::testing::_, ::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(Return());

  // start pressing
  expectUpdateButtons(h, timestamp, btn1, btn2, btn3);
  loop();
  // register press of required time
  for (int i = 0; i < iterations; ++i) {
    timestamp += iteration_time;
    expectUpdateButtons(h, timestamp, btn1, btn2, btn3);
    loop();
  }
  // release buttons
  timestamp += 10;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  ::testing::Mock::VerifyAndClearExpectations(&d);
  ::testing::Mock::VerifyAndClearExpectations(&h);
  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);
}

TEST(gui_test, main_screen) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);
  expectUpdateButtons(h, 123, false, false, false);
  expectMainScreen(d, 0);
  loop();
}

TEST(gui_test, add_counter_plus1) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  constexpr int start_time = 321;
  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);
  // start pressing the +1/-1 button
  expectUpdateButtons(h, start_time, true, false, false);
  expectMainScreen(d, 0);
  loop();

  expectUpdateButtons(h, start_time + 49, true, false, false);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, 0.049, -1, -1);
  loop();

  expectUpdateButtons(h, start_time + 50, true, false, false);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, 0.05, -1, -1);
  loop();

  // release +1/-1 button, press ok/drop button
  expectUpdateButtons(h, start_time + 60, false, false, true);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, -1);
  loop();

  expectUpdateButtons(h, start_time + 60 + 1, false, false, true);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, -1);
  loop();

  expectUpdateButtons(h, start_time + 60 + 1 + 100, false, false, true);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, 0.1);
  loop();

  // release ok/drop button
  expectUpdateButtons(h, start_time + 60 + 110, false, false, false);
  expectMainScreen(d, 1);
  expectMainScreenHistory(d, {"1.+1"});
  expectMainScreenButtonAnimation(d, -1, -1, -1);
  loop();
}

TEST(gui_test, add_counter_plus1_minus5) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);

  int timestamp = 321;
  // pressing the +1/-1 button
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);

  // press +5/-5 button
  expectUpdateButtons(h, timestamp + 60, false, true, false);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, -1);
  loop();

  expectUpdateButtons(h, timestamp + 60 + 150, false, true, false);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, 0.15, -1);
  loop();

  expectUpdateButtons(h, timestamp + 60 + 1000, false, true, false);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, 1, -1);
  loop();

  // release +5/-5 button, press ok/drop button
  expectUpdateButtons(h, timestamp + 60 + 1001, false, false, true);
  expectDeltaScreen(d, 0, -4);
  expectDeltaScreenButtonAnimation(d, -1, -1, -1);
  loop();

  expectUpdateButtons(h, timestamp + 60 + 1001 + 100, false, false, true);
  expectDeltaScreen(d, 0, -4);
  expectDeltaScreenButtonAnimation(d, -1, -1, 0.1);
  loop();

  // release ok/drop button
  expectUpdateButtons(h, timestamp + 60 + 1001 + 101, false, false, false);
  expectMainScreen(d, -4);
  expectMainScreenHistory(d, {"1.-4"});
  expectMainScreenButtonAnimation(d, -1, -1, -1);
  loop();
}

TEST(gui_test, go_to_menu) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);

  constexpr int start_time = 321;
  // start pressing the menu button
  expectUpdateButtons(h, start_time, false, false, true);
  expectMainScreen(d, 0);
  loop();

  expectUpdateButtons(h, start_time + 49, false, false, true);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, -1, -1, -1);
  loop();

  expectUpdateButtons(h, start_time + 50, false, false, true);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, -1, -1, 1);
  loop();

  // release the menu button
  expectUpdateButtons(h, start_time + 60, false, false, false);
  expectMenuScreen(d, 0);
  expectMenuScreenButtonAnimation(d, false, false, false);
  loop();

  // press back button
  expectUpdateButtons(h, start_time + 70, false, false, true);
  expectMenuScreen(d, 0);
  expectMenuScreenButtonAnimation(d, false, false, false);
  loop();

  expectUpdateButtons(h, start_time + 70 + 50, false, false, true);
  expectMenuScreen(d, 0);
  expectMenuScreenButtonAnimation(d, false, false, true);
  loop();

  expectUpdateButtons(h, start_time + 70 + 50 + 1000, false, false, true);
  expectMenuScreen(d, 0);
  expectMenuScreenButtonAnimation(d, false, false, true);
  loop();

  // release back button
  expectUpdateButtons(h, start_time + 70 + 50 + 1000 + 10, false, false, false);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, -1, -1, -1);
  loop();
}

TEST(gui_test, full_history) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);

  int timestamp = 321;
  // start pressing the menu button

  // add +1; +2; +3...
  int num_records = 10;
  for (int i = 0; i < num_records; ++i) {
    for (int j = 0; j < i + 1; ++j)
      pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1,
                                         timestamp);
    pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1,
                                       timestamp);
  }

  // goto menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  // goto history item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);

  // goto history menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  // check history contents
  expectHistoryScreen(d, {"1. 1=0+1", "2. 3=1+2", "3. 6=3+3", "4. 10=6+4",
                          "5. 15=10+5", "6. 21=15+6"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // scroll down one item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);

  // check scrolled contents
  expectHistoryScreen(d, {"2. 3=1+2", "3. 6=3+3", "4. 10=6+4", "5. 15=10+5",
                          "6. 21=15+6", "7. 28=21+7"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // scroll up one item
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);

  // check history contents
  expectHistoryScreen(d, {"1. 1=0+1", "2. 3=1+2", "3. 6=3+3", "4. 10=6+4",
                          "5. 15=10+5", "6. 21=15+6"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // scroll by continuous press
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 12, timestamp);
  expectHistoryScreen(d, {"5. 15=10+5", "6. 21=15+6", "7. 28=21+7",
                          "8. 36=28+8", "9. 45=36+9", "10. 55=45+10"});

  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // go back to manu screen
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);
  expectMenuScreen(d, 1);
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();
}

TEST(gui_test, delete_history) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);

  int timestamp = 321;
  // start pressing the menu button

  // add +1; +2; +3...
  int num_records = 10;
  for (int i = 0; i < num_records; ++i) {
    for (int j = 0; j < i + 1; ++j)
      pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1,
                                         timestamp);
    pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1,
                                       timestamp);
  }

  // goto menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  // goto "delete history" item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);

  // click "delete history"
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  // confirm "delete history"
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);

  // move up in menu to history item
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);

  // got to history
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  expectHistoryScreen(d, {});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // move to the menu and main screen
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  expectMainScreen(d, 0);
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // add an item to the history
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  expectMainScreen(d, 1);
  expectMainScreenHistory(d, {"1.+1"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();
}

TEST(gui_test, new_count) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);

  int timestamp = 321;
  // start pressing the menu button

  // add +1; +2; +3...
  int num_records = 3;
  for (int i = 0; i < num_records; ++i) {
    for (int j = 0; j < i + 1; ++j)
      pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1,
                                         timestamp);
    pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1,
                                       timestamp);
  }

  // goto menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  // goto "new count" item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, 1, timestamp);

  // click "new count"
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  // confirm
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);

  // move up in menu
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);

  // got to history
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  expectHistoryScreen(d, {"1. 1=0+1", "2. 3=1+2", "3. 6=3+3", "------"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // move to the menu and main screen
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, 1, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, 1, timestamp);

  expectMainScreen(d, 0);
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();
}

TEST(state_test, battery_state) {
  BatteryState bs;
  const float vcc = 3.3;
  const float bat_v = 1.4;
  const float tolerance = 0.001;

  // detecting no battery

  ASSERT_NEAR(bs.convertProbeValToState(0), -1.0f, tolerance);

  // detecting one battery
  bs.init(12, vcc);
  ASSERT_NEAR(bs.convertProbeValToState(1024), 1024.0 / 4096.0 * vcc / bat_v,
              tolerance);
  ASSERT_NEAR(bs.convertProbeValToState(1600), 1600.0 / 4096.0 * vcc / bat_v,
              tolerance);
  ASSERT_NEAR(bs.convertProbeValToState(2000), 1.0f, tolerance);

  // detecting two batteries
  bs.init(12, vcc);
  ASSERT_NEAR(bs.convertProbeValToState(2600),
              2600.0 / 4096.0 * vcc / (2 * bat_v), tolerance);
  ASSERT_NEAR(bs.convertProbeValToState(3000),
              3000.0 / 4096.0 * vcc / (2 * bat_v), tolerance);
  ASSERT_NEAR(bs.convertProbeValToState(3500), 1.0f, tolerance);
}

TEST(state_test, repeating_button_test) {
  RepeatingButtonState state(800, 200);
  auto checkTimestamp = [&state](long time, bool pressed, int expectedEvent) {
    auto event = state.updateState(time, pressed);
    ASSERT_EQ(event, expectedEvent);
    ASSERT_EQ(state.getState(), expectedEvent);
  };

  checkTimestamp(100, false, -1);
  checkTimestamp(200, true, 1);
  checkTimestamp(400, true, 0);
  checkTimestamp(999, true, 0);
  checkTimestamp(1000, true, 1);
  checkTimestamp(1000, true, 0);
  checkTimestamp(1001, true, 0);
  checkTimestamp(1199, true, 0);
  checkTimestamp(1200, true, 1);
  checkTimestamp(1200, true, 0);
  checkTimestamp(1201, true, 0);
  checkTimestamp(1400, true, 1);
  checkTimestamp(1600, false, -1);
}

TEST(state_test, persisten_state_test) {
  PersistentMemory raw_mem(true, 32);
  PersistentMemoryWrapper mem(&raw_mem, 32);
  mem.setup();
  PersistentState s1(&mem);
  s1.restoreFromMem([](int, int) { FAIL(); }, []() { FAIL(); },
                    []() { FAIL(); });
  for (int i = 0; i < 5; ++i)
    s1.rememberNewValue(1 << i);
  s1.rememberStartNewCount();
  s1.rememberStartNewCount();
  s1.rememberNewValue(32);
  s1.rememberClearHistory();
  PersistentState s2(&mem);
  int sum = 0;
  int new_counts = 0;
  int resets = 0;
  s2.restoreFromMem([&](int value, int delta) { sum += value; },
                    [&]() { resets++; }, [&]() { new_counts++; });
  ASSERT_EQ(sum, 63);
  ASSERT_EQ(new_counts, 2);
  ASSERT_EQ(resets, 1);
}

TEST(state_test, persisten_state_overflow_test) {
  PersistentMemory raw_mem(true, 16);
  PersistentMemoryWrapper mem(&raw_mem, 16);
  mem.setup();
  PersistentState s1(&mem);
  s1.restoreFromMem([](int, int) { FAIL(); }, []() { FAIL(); },
                    []() { FAIL(); });
  for (int i = 0; i < 10; ++i)
    s1.rememberNewValue(1 << i);
  PersistentState s2(&mem);
  int sum = 0;
  int new_counts = 0;
  int resets = 0;
  s2.restoreFromMem([&](int value, int delta) { sum += value; },
                    [&]() { resets++; }, [&]() { new_counts++; });
  ASSERT_EQ(sum, 992);
  ASSERT_EQ(new_counts, 0);
  ASSERT_EQ(resets, 0);
}

TEST(state_test, invalid_persisten_state_test) {
  PersistentMemory raw_mem(false, 32);
  PersistentMemoryWrapper mem(&raw_mem, 32);
  mem.setup();
  PersistentState s1(&mem);
  s1.restoreFromMem([](int, int) { FAIL(); }, []() { FAIL(); },
                    []() { FAIL(); });
  for (int i = 0; i < 5; ++i)
    s1.rememberNewValue(1 << i);
  s1.rememberStartNewCount();
  s1.rememberClearHistory();
  s1.rememberNewValue(32);
  PersistentState s2(&mem);
  s2.restoreFromMem([](int, int) { FAIL(); }, []() { FAIL(); },
                    []() { FAIL(); });
}

TEST(widget_test, menu_wrap_navigation) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  ListWithSelectorWidget<5> list;
  list.setParams(100, CHAR_H * 4, 0);
  list.addItem("item 0");
  list.addItem("item 1");
  list.addItem("item 2");
  list.addItem("item 3");
  list.addItem("item 4");
  list.setPos(&h, 0, 0);

  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  ASSERT_EQ(list.getSelPos(), 0);
  list.moveSelDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  ASSERT_EQ(list.getSelPos(), 1);
  list.moveSelDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  ASSERT_EQ(list.getSelPos(), 2);
  list.moveSelDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  ASSERT_EQ(list.getSelPos(), 3);
  list.moveSelDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  ASSERT_EQ(list.getSelPos(), 4);
  list.moveSelDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  ASSERT_EQ(list.getSelPos(), 0);
  list.moveSelUp();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  ASSERT_EQ(list.getSelPos(), 4);
  list.moveSelUp();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  ASSERT_EQ(list.getSelPos(), 3);
  list.moveSelUp();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  ASSERT_EQ(list.getSelPos(), 2);
  list.moveSelUp();
  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  ASSERT_EQ(list.getSelPos(), 1);
}

TEST(widget_test, history_wrap_navigation) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  OverwritingListWidget<5> list;
  list.setParams(100, CHAR_H * 4);
  list.addItem("item 0");
  list.addItem("item 1");
  list.addItem("item 2");
  list.addItem("item 3");
  list.addItem("item 4");
  list.setPos(&h, 0, 0);

  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  list.moveDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  list.moveDown();
  ASSERT_EQ(list.getFirstVisibleItem(), 0);
  list.moveUp();
  ASSERT_EQ(list.getFirstVisibleItem(), 1);
  list.moveUp();
  ASSERT_EQ(list.getFirstVisibleItem(), 0);
}

TEST(widget_test, repeating_button) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  RepeatingButtonWidget btn;
  btn.setParams("test", 0 /*physical button id*/, [](int event) {});
  btn.setPos(&h, 0, 0);
  auto checkDraw = [&](long time, bool pressed) {
    expectUpdateButtons(h, time, pressed, false, false);
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq("test"))));
    EXPECT_CALL(d, setTextSize(1));
    EXPECT_CALL(d, setTextColor(1));
    EXPECT_CALL(d, setCursor(0, 0));
    if (pressed)
      EXPECT_CALL(d, drawFastHLine(0, CHAR_H, 4 * CHAR_W, Color::WHITE));
    btn.update();
    btn.draw();
  };
  checkDraw(0, false);
  for (int time : {100, 500, 799, 800, 801, 999, 1000, 1001, 1200, 1401})
    checkDraw(time, true);
  checkDraw(1402, false);
}

TEST(screen_test, main_screen_history) {
  Display d;
  PersistentMemory pm(true, 1024);
  PersistentMemoryWrapper mem(&pm, 1024);
  HAL h(&d, &mem);
  EXPECT_CALL(d, width()).WillRepeatedly(Return(128));
  EXPECT_CALL(d, height()).WillRepeatedly(Return(64));
  EXPECT_CALL(d, setTextColor(::testing::_)).WillRepeatedly(Return());
  EXPECT_CALL(d, setTextSize(::testing::_)).WillRepeatedly(Return());
  EXPECT_CALL(d, setCursor(::testing::_, testing::_)).WillRepeatedly(Return());
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("item 4"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("item 5"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("item 6"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("item 7"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("item 8"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("item 9"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+1/-1"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("0"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+5/-5"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("menu"))));
  counter_gui::MainScreen screen;
  screen.setup(
      &h, [](int) {}, [](int) {}, [](int) {});
  for (int i = 0; i < 10; ++i)
    screen.addHistoryItem(("item " + std::to_string(i)).c_str());
  screen.draw();
}

#endif
