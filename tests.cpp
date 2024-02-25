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

TEST(basic_tests, main_menu_draw) {
  Display d;
  HAL h(&d);

  EXPECT_CALL(h, buttonPressed(0));
  EXPECT_CALL(h, uptimeMillis()).Times(3);
  EXPECT_CALL(h, buttonPressed(1));
  EXPECT_CALL(h, buttonPressed(2));
  EXPECT_CALL(d, setTextColor(1)).Times(5);
  EXPECT_CALL(d, setCursor(0, 53));
  EXPECT_CALL(d, setTextSize(1)).Times(4);
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+1"))));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("/")))).Times(2);
  EXPECT_CALL(d, setCursor(18, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("-1"))));
  EXPECT_CALL(d, setCursor(34, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("+5"))));
  EXPECT_CALL(d, setCursor(52, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("-5"))));
  EXPECT_CALL(d, setCursor(104, 53));
  EXPECT_CALL(d, print(Matcher<const char *>(StrEq("menu"))));
  EXPECT_CALL(d, setCursor(0, 0));
  EXPECT_CALL(d, setTextSize(5));
  EXPECT_CALL(d, print(Matcher<int>(0)));

  counter_gui::setup(&h);
  counter_gui::loop();
}

#endif
