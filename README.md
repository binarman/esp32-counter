# esp32-counter

## Build

### Arduino IDE

This is release build for hardware.

Dependencies:
- ESP32 board support in arduino IDE
- https://github.com/adafruit/Adafruit_SH110X

### CMake test

This build is for debug purposes only.

Build and run tests:

```
sudo apt install libgtest-dev libgmock-dev
mkdir build
cd build
cmake ..
make
./counter_tests
```
