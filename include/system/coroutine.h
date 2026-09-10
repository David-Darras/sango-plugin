/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "common.h"

extern "C" void CoroutineSwitch(uptr* save_sp, uptr load_sp);

class Coroutine {
public:
  typedef void (*Function)(void* arg);

  Coroutine(u8* stack, u32 stack_size) : stack_(stack), stack_size_(stack_size) {}

  void Start(Function function, void* arg) {
    function_ = function;
    arg_ = arg;
    is_finished_ = false;
    is_started_ = false;
    WriteCanary();
    uptr top = ((uptr)stack_ + stack_size_) & ~(uptr)7;
    uptr* frame = (uptr*)(top - kFrameSize);
    for (u32 i = 0; i < kFrameWords - 1; i++) frame[i] = 0;
    frame[kFrameWords - 1] = (uptr)&Coroutine::Entry;
    stack_pointer_ = (uptr)frame;
  }

  bool Resume() {
    if (is_finished_ || function_ == nullptr) return false;
    if (!is_started_) {
      self_ = this;
      is_started_ = true;
    }
    CoroutineSwitch(&caller_stack_pointer_, stack_pointer_);
    return !is_finished_;
  }

  void Yield() { CoroutineSwitch(&stack_pointer_, caller_stack_pointer_); }

  INLINE bool IsFinished() const { return is_finished_; }
  INLINE bool IsStarted() const { return is_started_; }
  INLINE bool IsStackHealthy() const {
    return *(const u32*)stack_ == kCanary;
  }
  INLINE u32 GetStackHighWater() const {
    const u32* words = (const u32*)stack_;
    u32 untouched = 0;
    for (u32 i = 1; i < stack_size_ / sizeof(u32); i++) {
      if (words[i] != kCanary) break;
      untouched++;
    }
    return stack_size_ - untouched * sizeof(u32);
  }

private:
  static constexpr u32 kFrameWords = 16 + 8 + 1;
  static constexpr u32 kFrameSize = kFrameWords * sizeof(uptr);
  static constexpr u32 kCanary = 0xC0A0B0D0;

  void WriteCanary() {
    u32* words = (u32*)stack_;
    for (u32 i = 0; i < stack_size_ / sizeof(u32); i++) words[i] = kCanary;
  }

  static void Entry() {
    Coroutine* self = self_;
    self->function_(self->arg_);
    self->is_finished_ = true;
    for (;;) self->Yield();
  }

  static Coroutine* self_;

  u8* stack_;
  u32 stack_size_;
  Function function_ = nullptr;
  void* arg_ = nullptr;
  uptr stack_pointer_ = 0;
  uptr caller_stack_pointer_ = 0;
  bool is_finished_ = true;
  bool is_started_ = false;
};
