#ifdef TEST_MODE

#include "counter_gui.h"
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

void expectMenuScreen(Display &d) {
  EXPECT_CALL(d, setTextColor(1)).Times(4);
  EXPECT_CALL(d, setTextSize(1)).Times(4);
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setCursor(0, CHAR_H));
  EXPECT_CALL(d, setCursor(0, CHAR_H * 2));
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setCursor(61, 53));
  EXPECT_CALL(d, setCursor(80, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1ashow full history"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq(" start new counting"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq(" drop full history"))));

  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1e"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("\x1f"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("sel/back"))));
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

void expectMenuScreenButtonAnimation(Display &d, float btn1, float btn2,
                                     float btn3) {
  if (btn1 >= 0)
    assert(false && "todo implemnet");

  if (btn2 >= 0)
    assert(false && "todo implement");

  if (btn3 >= 0)
    EXPECT_CALL(d,
                drawFastHLine(80, 63, (int)(btn3 * 8 * CHAR_W), Color::WHITE));
  if (btn3 == 1.0)
    EXPECT_CALL(d,
                drawFastHLine(80 + 4 * CHAR_W, 61, 4 * CHAR_W, Color::WHITE));
  else if (btn3 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(80, 61, 3 * CHAR_W, Color::WHITE));
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
                                        int press_time, int &timestamp) {
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
  timestamp += press_time;
  expectUpdateButtons(h, timestamp, btn1, btn2, btn3);
  loop();
  // release buttons
  timestamp += 10;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  ::testing::Mock::VerifyAndClearExpectations(&d);
  ::testing::Mock::VerifyAndClearExpectations(&h);
  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);
}

TEST(basic_tests, main_screen) {
  Display d;
  HAL h(&d);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);
  expectUpdateButtons(h, 123, false, false, false);
  expectMainScreen(d, 0);
  loop();
}

TEST(basic_tests, add_counter_plus1) {
  Display d;
  HAL h(&d);
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

TEST(basic_tests, add_counter_plus1_minus5) {
  Display d;
  HAL h(&d);
  expectSetup(h);
  counter_gui::setup(&h);

  expectBatteryDraw(d);
  expectBatteryState(h, 0.5);

  int timestamp = 321;
  // pressing the +1/-1 button
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);

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

TEST(basic_tests, go_to_menu) {
  Display d;
  HAL h(&d);
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
  expectMenuScreen(d);
  expectMenuScreenButtonAnimation(d, -1, -1, -1);
  loop();

  // press back button
  expectUpdateButtons(h, start_time + 70, false, false, true);
  expectMenuScreen(d);
  expectMenuScreenButtonAnimation(d, -1, -1, -1);
  loop();

  expectUpdateButtons(h, start_time + 70 + 50, false, false, true);
  expectMenuScreen(d);
  expectMenuScreenButtonAnimation(d, -1, -1, 0.05);
  loop();

  expectUpdateButtons(h, start_time + 70 + 50 + 1000, false, false, true);
  expectMenuScreen(d);
  expectMenuScreenButtonAnimation(d, -1, -1, 1);
  loop();

  // release back button
  expectUpdateButtons(h, start_time + 70 + 50 + 1000 + 10, false, false, false);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, -1, -1, -1);
  loop();
}

TEST(basic_tests, full_history) {
  Display d;
  HAL h(&d);
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
      pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);
    pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);
  }

  // goto menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  // goto history
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  // check history contents
  expectHistoryScreen(d, {"1. 1=0+1", "2. 3=1+2", "3. 6=3+3", "4. 10=6+4",
                          "5. 15=10+5", "6. 21=15+6"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // scroll down one item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, timestamp);

  // check scolled contents
  expectHistoryScreen(d, {"2. 3=1+2", "3. 6=3+3", "4. 10=6+4", "5. 15=10+5",
                          "6. 21=15+6", "7. 28=21+7"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // scroll up one item
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);

  // check history contents
  expectHistoryScreen(d, {"1. 1=0+1", "2. 3=1+2", "3. 6=3+3", "4. 10=6+4",
                          "5. 15=10+5", "6. 21=15+6"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // go back to manu screen
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);
  expectMenuScreen(d);
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();
}

TEST(basic_tests, delete_history) {
  Display d;
  HAL h(&d);
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
      pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);
    pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);
  }

  // goto menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  // goto "delete history" item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, timestamp);

  // click "delete history"
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  // confirm "delete history"
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);

  // move up in menu
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);

  // got to history
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  expectHistoryScreen(d, {});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // move to the menu and main screen
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 1000, timestamp);

  expectMainScreen(d, 0);
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // add an item to the history
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  expectMainScreen(d, 1);
  expectMainScreenHistory(d, {"1.+1"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();
}

TEST(basic_tests, new_count) {
  Display d;
  HAL h(&d);
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
      pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);
    pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);
  }

  // goto menu
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  // goto "new count" item
  pressAndReleaseButtonsIgnoreOutput(h, false, true, false, 100, timestamp);

  // click "new count"
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  // confirm
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);

  // move up in menu
  pressAndReleaseButtonsIgnoreOutput(h, true, false, false, 100, timestamp);

  // got to history
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);

  expectHistoryScreen(d, {"1. 1=0+1", "2. 3=1+2", "3. 6=3+3", "------"});
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();

  // move to the menu and main screen
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 100, timestamp);
  pressAndReleaseButtonsIgnoreOutput(h, false, false, true, 1000, timestamp);

  expectMainScreen(d, 0);
  timestamp += 1;
  expectUpdateButtons(h, timestamp, false, false, false);
  loop();
}

TEST(basic_tests, battery_state) {
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

#endif
