#ifndef STATE_H
#define STATE_H

#include "hal.h"

class PersistentState {
  PersistentMemoryWrapper *mem = nullptr;
  int sequence_end = 0;

  void putEndMark(int offset);

public:
  PersistentState() = default;
  PersistentState(PersistentMemoryWrapper *m) : mem(m) {}

  void setup(PersistentMemoryWrapper *m) { mem = m; }

  void restoreFromMem(std::function<void(int, int)> onChange,
                      std::function<void()> onClearHistory,
                      std::function<void()> onNewCount);

  void rememberNewValue(int value);

  void rememberClearHistory();

  void rememberStartNewCount();
};

#endif // STATE_H