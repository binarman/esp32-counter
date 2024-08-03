#include "state.h"
#include <memory>

#define WRAP(offset) ((offset) % mem_size)

namespace {
constexpr int end_mark_size = 3;

bool isEndMark(uint8_t *mem, int pos, int mem_size) {
  if (mem[WRAP(pos)] != 0xff)
    return false;
  if (mem[WRAP(pos + 1)] != 0)
    return false;
  if (mem[WRAP(pos + 2)] != 0)
    return false;
  return true;
}
} // namespace

void PersistentState::putEndMark(int offset) {
  int mem_size = mem->size();
  assert(offset >= 0 && offset < mem_size);
  for (int i = 0; i < end_mark_size; ++i) {
    int rep_offset = WRAP(offset + i);
    switch (mem->read(rep_offset)) {
    case 1:
      for (int j = 0; j < 3; ++j)
        mem->write(WRAP(rep_offset + j), 0);
      break;
    default:
      mem->write(rep_offset, 0);
      break;
    }
  }
  mem->write(offset, 0xff);
  mem->write(WRAP(offset + 1), 0x00);
  mem->write(WRAP(offset + 2), 0x00);
}

void PersistentState::restoreFromMem(std::function<void(int, int)> onChange,
                                     std::function<void()> onClearHistory,
                                     std::function<void()> onNewCount) {
  if (!mem->isValid())
    return;
  int mem_size = mem->size();
  std::unique_ptr<uint8_t[]> buffer{new uint8_t[mem_size]};

  for (int i = 0; i < mem_size; ++i)
    buffer[i] = mem->read(i);

  // find zero
  // consider zero as a sequence start replaying events
  sequence_end = -1;
  for (int i = 0; i < mem_size; ++i)
    if (isEndMark(buffer.get(), i, mem_size)) {
      sequence_end = i;
      break;
    }
  if (sequence_end == -1) {
    // mem is in inconsistent state, initialize with zeros
    putEndMark(0);
    for (int i = 3; i < mem_size; ++i)
      mem->write(i, 0);
    sequence_end = 0;
    return;
  }
  assert(sequence_end >= 0 && sequence_end < mem_size);
  int value = 0;
  for (int offset = 3; offset < mem_size;) {
    auto commandId = buffer[WRAP(sequence_end + offset)];
    // 1 add new number
    // 2 clear history
    // 3 start new count
    // all others are ignored
    switch (commandId) {
    case 1: {
      int16_t new_value = buffer[WRAP(sequence_end + offset + 1)] |
                          (buffer[WRAP(sequence_end + offset + 2)] << 8);
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

void PersistentState::rememberNewValue(int value) {
  if (!mem->isValid())
    return;
  int mem_size = mem->size();
  assert(sequence_end >= 0 && sequence_end < mem_size);
  putEndMark(WRAP(sequence_end + 3));
  mem->write(sequence_end, 1);
  mem->write(WRAP(sequence_end + 1), value & 0xff);
  mem->write(WRAP(sequence_end + 2), (value >> 8) & 0xff);
  sequence_end = WRAP(sequence_end + 3);
}

void PersistentState::rememberClearHistory() {
  if (!mem->isValid())
    return;
  int mem_size = mem->size();
  assert(sequence_end >= 0 && sequence_end < mem_size);
  putEndMark(WRAP(sequence_end + 1));
  mem->write(sequence_end, 2);
  sequence_end = WRAP(sequence_end + 1);
}

void PersistentState::rememberStartNewCount() {
  if (!mem->isValid())
    return;
  int mem_size = mem->size();
  assert(sequence_end >= 0 && sequence_end < mem_size);
  putEndMark(WRAP(sequence_end + 1));
  mem->write(sequence_end, 3);
  sequence_end = WRAP(sequence_end + 1);
}
