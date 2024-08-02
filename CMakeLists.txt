cmake_minimum_required(VERSION 3.10)

project(esp32-counter)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC tests.cpp counter_gui.cpp hal.cpp state.cpp)
set(HDR counter_gui.h screens.h widgets.h hal.h state.h)

find_package(GTest REQUIRED)

enable_testing()

add_executable(counter_tests ${SRC})
target_compile_definitions(counter_tests PUBLIC TEST_MODE)
target_link_libraries(counter_tests gtest gmock gmock_main)

target_compile_options(counter_tests PRIVATE -fsanitize=address)
target_link_options(counter_tests PRIVATE -fsanitize=address)

include(GoogleTest)
gtest_discover_tests(counter_tests)
