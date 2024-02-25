#ifdef TEST_MODE

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "counter_gui.h"

using ::testing::An;
using ::testing::Matcher;
using ::testing::TypedEq;
using ::testing::Return;
using ::testing::_;

TEST(HelloTest, BasicAssertions) {
  Display d;
  HAL h(&d);
  counter_gui::setup(&h);
  counter_gui::loop();
}

#endif
