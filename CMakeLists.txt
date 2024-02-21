project(esp32-counter)

cmake_minimum_required(VERSION 3.10)

set(SRC tests.cpp counter_gui.cpp test_hal.cpp)
set(HDR counter_gui.h widgets.h hal.h)

add_executable(counter_tests ${SRC})
target_compile_definitions(counter_tests PUBLIC TEST_MODE)

