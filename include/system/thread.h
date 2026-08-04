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
#include "game/manager.h"
#include "ui/log_application.h"

struct Event {
  s32 counter = 0;
  s32 lock = 0;
};

class Thread {
public:
  Thread() {
    uptr heap = (uptr)game::Manager::GetInstance().GetSystemHeap();
    u32 stack_size = 0x400;
    ((void(*)(Thread*, uptr, u32, u8))0x0014E424)(this, heap, stack_size, 0);
  }

  void Start(u8 priority = 8) {
    ((void(*)(Thread*, u8))0x00139C5C)(this, priority);
  }

  virtual ~Thread() {
    ((void(*)(Thread*))0x003A8174)(this);
  }

  virtual void Initialize() {
  }

  virtual void Update() = 0;

  virtual void Finalize() {
  }

  virtual void OnKill() {
  }

private:
  // void* vtable;
  uptr handle_;
  bool use_finalize_;
  bool use_auto_stack_;
  bool _0[2];
  uptr basic_stack_area_;
  uptr safe_stack_area_;
  Event pause;
  Event wait;
  bool is_paused_;
  bool ask_to_pause_;
  bool ask_to_kill_;
  bool _1;
};