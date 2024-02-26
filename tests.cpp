#ifdef TEST_MODE

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "counter_gui.h"

using ::testing::An;
using ::testing::Matcher;
using ::testing::TypedEq;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::_;

void expectUpdateButtons(HAL &h, int timestamp, bool btn1, bool btn2, bool btn3) {
  EXPECT_CALL(h, uptimeMillis()).WillOnce(Return(timestamp)).WillOnce(Return(timestamp)).WillOnce(Return(timestamp));
  EXPECT_CALL(h, buttonPressed(0)).WillOnce(Return(btn1));
  EXPECT_CALL(h, buttonPressed(1)).WillOnce(Return(btn2));;
  EXPECT_CALL(h, buttonPressed(2)).WillOnce(Return(btn3));;
}

void expectMainScreen(Display &d, int counter) {
  EXPECT_CALL(d, setTextColor(1)).Times(5);
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setTextSize(1)).Times(4);
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+1/-1"))));
  EXPECT_CALL(d, setCursor(34, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+5/-5"))));
  EXPECT_CALL(d, setCursor(104, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("menu"))));
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setTextSize(5));
  EXPECT_CALL(d, print(Matcher<int>(counter)));
}

void expectMainScreenHistory(Display &d, const std::vector<std::string> &entries) {
  for (int i = 0; i < entries.size(); ++i){
    EXPECT_CALL(d, setCursor(72, i*8));
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq(entries[i]))));
  }
}

void expectDeltaScreen(Display &d, int counter, int delta) {
  EXPECT_CALL(d, setTextColor(1)).Times(4);
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setTextSize(1)).Times(4);

  EXPECT_CALL(d, setCursor(90, 0));
  if (delta >= 0)
    EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+"))));
  EXPECT_CALL(d, setCursor(90, 8));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("="))));
  if (counter == 0) {
    EXPECT_CALL(d, print(Matcher<int>(delta))).Times(2);
  } else {
    EXPECT_CALL(d, print(Matcher<int>(delta)));
    EXPECT_CALL(d, print(Matcher<int>(counter+delta)));
  }

  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+1/-1"))));
  EXPECT_CALL(d, setCursor(34, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+5/-5"))));
  EXPECT_CALL(d, setCursor(86, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("ok/drop"))));
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setTextSize(5));
  EXPECT_CALL(d, print(Matcher<int>(counter)));
}

void expectMainScreenButtonAnimation(Display &d, float btn1, float btn2, float btn3) {
  if (btn1 >= 0)
    EXPECT_CALL(d, drawFastHLine(0, 63, (int)(btn1*5*6), SH110X_WHITE));
  if (btn1 == 1.0)
    EXPECT_CALL(d, drawFastHLine(3*6, 61, 2*6, SH110X_WHITE));
  else if (btn1 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(0, 61, 2*6, SH110X_WHITE));

  if (btn2 >= 0)
    assert(false && "todo implement");

  if (btn3 >= 0)
    assert(false && "todo implement");
}

void expectDeltaScreenButtonAnimation(Display &d, float btn1, float btn2, float btn3) {
  if (btn1 >= 0)
    EXPECT_CALL(d, drawFastHLine(0, 63, (int)(btn1*5*6), SH110X_WHITE));
  if (btn1 == 1.0)
    EXPECT_CALL(d, drawFastHLine(3*6, 61, 2*6, SH110X_WHITE));
  else if (btn1 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(0, 61, 2*6, SH110X_WHITE));

  if (btn2 >= 0)
    assert(false && "todo implement");

  if (btn3 >= 0)
    EXPECT_CALL(d, drawFastHLine(86, 63, (int)(btn3*7*6), SH110X_WHITE));
  if (btn3 == 1.0)
    assert(false && "todo implement");
  else if (btn3 >= 0.05)
    EXPECT_CALL(d, drawFastHLine(86, 61, 2*6, SH110X_WHITE));
}

TEST(basic_tests, main_screen) {
  Display d;
  HAL h(&d);
  expectUpdateButtons(h, 123, false, false, false);
  expectMainScreen(d, 0);
  counter_gui::setup(&h);
  counter_gui::loop();
}

TEST(basic_tests, add_counter) {
  Display d;
  HAL h(&d);
  constexpr int start_time = 321;
  // start pressing the +1/-1 button
  expectUpdateButtons(h, start_time, true, false, false);
  expectMainScreen(d, 0);
  counter_gui::setup(&h);
  counter_gui::loop();

  expectUpdateButtons(h, start_time + 49, true, false, false);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, 0.049, -1, -1);
  counter_gui::loop();

  expectUpdateButtons(h, start_time + 50, true, false, false);
  expectMainScreen(d, 0);
  expectMainScreenButtonAnimation(d, 0.05, -1, -1);
  counter_gui::loop();

  // release +1/-1 button, press ok/drop button
  expectUpdateButtons(h, start_time + 60, false, false, true);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, -1);
  counter_gui::loop();

  expectUpdateButtons(h, start_time + 60 + 1, false, false, true);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, -1);
  counter_gui::loop();

  expectUpdateButtons(h, start_time + 60 + 1 + 100, false, false, true);
  expectDeltaScreen(d, 0, 1);
  expectDeltaScreenButtonAnimation(d, -1, -1, 0.1);
  counter_gui::loop();

  // release ok/drop button
  expectUpdateButtons(h, start_time + 60 + 110, false, false, false);
  expectMainScreen(d, 1);
  expectMainScreenHistory(d, {"1.+1"});
  expectMainScreenButtonAnimation(d, -1, -1, -1);
  counter_gui::loop();
}

#endif
