#ifndef STATE_H
#define STATE_H

#include "hal.h"

class PersistentState {
  PersistentMemoryWrapper *mem = nullptr;
  int sequence_end;

public:
  PersistentState() = default;
  PersistentState(PersistentMemoryWrapper *m) : mem(m) {}

  void setup(PersistentMemoryWrapper *m) { mem = m; }

  void restoreFromMem(std::function<void(int, int)> onChange,
                      std::function<void()> onClearHistory,
                      std::function<void()> onNewCount);

  /**
   * @param offset first offset to clear
   * @param repeats minimum number of bytest to clear
   */
  void eraseCommands(int offset, int repeats = 1);

  void rememberNewValue(int value);

  void rememberClearHistory();

  void rememberStartNewCount();
};

#endif // STATE_H