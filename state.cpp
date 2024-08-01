#include "state.h"

void PersistentState::restoreFromMem(std::function<void(int, int)> onChange,
                                     std::function<void()> onClearHistory,
                                     std::function<void()> onNewCount) {
  if (!mem->isValid())
    return;
  int buffer_size = mem->size();
  // find zero
  // consider zero as a sequence start replaying events
  sequence_end = -1;
  for (int i = 0; i < buffer_size; ++i)
    if (mem->read(i) == 0) {
      sequence_end = i;
      break;
    }
  if (sequence_end == -1) {
    // mem is in inconsistent state, initialize with zeros
    for (int i = 0; i < buffer_size; ++i)
      mem->write(i, 0);
    return;
  }
  assert(sequence_end >= 0 && sequence_end < buffer_size);
  int value = 0;
  for (int offset = 0; offset < buffer_size;) {
    auto commandId = mem->read((sequence_end + offset) % buffer_size);
    // 1 add new number
    // 2 clear history
    // 3 start new count
    // all others are ignored
    switch (commandId) {
    case 1: {
      int16_t new_value =
          mem->read((sequence_end + offset + 1) % buffer_size) |
          (mem->read((sequence_end + offset + 2) % buffer_size) << 8);
      int delta = new_value - value;
      value = new_value;
      onChange(new_value, delta);
      offset += 3;
      break;
    }
    case 2:
      onClearHistory();
      value = 0;
      offset++;
      break;
    case 3:
      onNewCount();
      value = 0;
      offset++;
      break;
    default:
      value = 0;
      offset++;
      break;
    }
  }
}

/**
 * @param offset first offset to clear
 * @param repeats minimum number of bytest to clear
 */
void PersistentState::eraseCommands(int offset, int repeats) {
  int buffer_size = mem->size();
  for (int i = 0; i < repeats; ++i) {
    int rep_offset = (offset + 1) % buffer_size;
    switch (mem->read(rep_offset)) {
    case 1:
      mem->write(rep_offset, 0);
      mem->write((rep_offset + 1) % buffer_size, 0);
      mem->write((rep_offset + 2) % buffer_size, 0);
      break;
    case 2:
    case 3:
      mem->write(rep_offset, 0);
      break;
    default:
      // memory contains garbage command id, ignore it
      break;
    }
  }
}

void PersistentState::rememberNewValue(int value) {
  if (!mem->isValid())
    return;
  int buffer_size = mem->size();
  eraseCommands(sequence_end + 1, 3);
  mem->write(sequence_end, 1);
  mem->write((sequence_end + 1) % buffer_size, value & 0xff);
  mem->write((sequence_end + 2) % buffer_size, (value >> 8) & 0xff);
  sequence_end = (sequence_end + 3) % buffer_size;
}

void PersistentState::rememberClearHistory() {
  if (!mem->isValid())
    return;
  int buffer_size = mem->size();
  eraseCommands(sequence_end + 1);
  mem->write(sequence_end, 2);
  sequence_end = (sequence_end + 1) % buffer_size;
}

void PersistentState::rememberStartNewCount() {
  if (!mem->isValid())
    return;
  int buffer_size = mem->size();
  eraseCommands(sequence_end + 1);
  mem->write(sequence_end, 3);
  sequence_end = (sequence_end + 1) % buffer_size;
  ;
}
