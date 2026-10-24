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

/**
 * @file frame_trace.cc
 * @brief The black box of the plugin. See core/frame_trace.h.
 */

#include "core/frame_trace.h"

#include <3ds.h>
#include <stdio.h>
#include <string.h>

#include "core/game_file.h"
#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "core/utils.h"
#include "system/native/file.h"
#include "system/native/graphics.h"
#include "ui/main_application.h"

namespace core {
namespace {
const c16* const kPath = u"sdmc:/sango/frame_trace.txt";
// The file always has this size: a new write replaces the old text.
constexpr u32 kFileSize = 1024;
// The frames between two writes: 3 writes each second at 60 frames.
constexpr u32 kWriteInterval = 20;

struct State {
  u32 frame;
  u32 command_bytes[2]; ///< The command list after the top / bottom screen.
  u32 texts_at[2]; ///< Texts + shadows of the frame after each screen.
  u32 rects_at[2];
  u32 peak_command_bytes[2];
  u32 peak_texts;
  u32 peak_rects;
  u32 min_linear_kb;
  u32 min_heap_kb[2];
  u32 opened_frames; ///< The frames with the menu open.
};
State state = {0, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, 0, 0xFFFFFFFF,
               {0xFFFFFFFF, 0xFFFFFFFF}, 0};

void Lower(u32& minimum, u32 value) {
  if (value < minimum) minimum = value;
}

void Raise(u32& maximum, u32 value) {
  if (value > maximum) maximum = value;
}
} // namespace

void FrameTrace::OnScreenDrawn(bool is_top) {
  const u32 index = is_top ? 0 : 1;
  state.command_bytes[index] = sys::Graphics::GetCommandBytes();
  Raise(state.peak_command_bytes[index], state.command_bytes[index]);
  const sys::Graphics::TextBudget& budget = sys::Graphics::GetTextBudget();
  state.texts_at[index] = budget.texts + budget.shadows;
  state.rects_at[index] = budget.rects;
}

void FrameTrace::EndFrame() {
  state.frame++;
  const sys::Graphics::TextBudget& budget = sys::Graphics::GetTextBudget();
  Raise(state.peak_texts, budget.texts + budget.shadows);
  Raise(state.peak_rects, budget.rects);

  auto& menu = ui::MainApplication::GetInstance();
  if (!menu.IsOpened()) return;
  state.opened_frames++;
  if (state.opened_frames % kWriteInterval != 1) return;

  // The free memory: the system call and the two heaps of the game.
  const u32 linear_kb = osGetMemRegionFree(MEMREGION_APPLICATION) / 1024;
  const u32 heap_kb = GameFile::GetFreeHeapMemory(0) / 1024;
  const u32 device_kb = GameFile::GetFreeHeapMemory(1) / 1024;
  Lower(state.min_linear_kb, linear_kb);
  Lower(state.min_heap_kb[0], heap_kb);
  Lower(state.min_heap_kb[1], device_kb);

  // Unmangle() returns the same buffer each time: copy the first name.
  uptr vtable = 0;
  c8 process_name[64];
  strncpy(process_name,
          Utils::Unmangle(ProcessManager::GetInstance().GetCurrentProcessName(
              vtable)),
          sizeof(process_name) - 1);
  process_name[sizeof(process_name) - 1] = '\0';
  const c8* event_name =
      Utils::Unmangle(EventManager::GetInstance().GetCurrentEventName(vtable));

  c8 where[160];
  menu.DescribeState(where, sizeof(where));

  c8 text[kFileSize];
  const int length = snprintf(
      text, sizeof(text),
      "frame %lu (menu open %lu)\n"
      "menu: %s\n"
      "process: %s | event: %s\n"
      "this frame: texts %lu + shadows %lu, rects %lu\n"
      "peak: texts+shadows %lu, rects %lu\n"
      "after top screen: texts+shadows %lu, rects %lu\n"
      "after bottom screen: texts+shadows %lu, rects %lu\n"
      "command list now (bytes): top %lu, bottom %lu\n"
      "command list peak (bytes): top %lu, bottom %lu\n"
      "free now (KB): linear %lu, heap %lu, device %lu\n"
      "free lowest (KB): linear %lu, heap %lu, device %lu\n",
      (unsigned long)state.frame, (unsigned long)state.opened_frames, where,
      process_name, event_name, (unsigned long)budget.texts,
      (unsigned long)budget.shadows, (unsigned long)budget.rects,
      (unsigned long)state.peak_texts, (unsigned long)state.peak_rects,
      (unsigned long)state.texts_at[0], (unsigned long)state.rects_at[0],
      (unsigned long)state.texts_at[1], (unsigned long)state.rects_at[1],
      (unsigned long)state.command_bytes[0],
      (unsigned long)state.command_bytes[1],
      (unsigned long)state.peak_command_bytes[0],
      (unsigned long)state.peak_command_bytes[1], (unsigned long)linear_kb,
      (unsigned long)heap_kb, (unsigned long)device_kb,
      (unsigned long)state.min_linear_kb, (unsigned long)state.min_heap_kb[0],
      (unsigned long)state.min_heap_kb[1]);

  // A text that is longer than the file is cut. A shorter text gets spaces,
  // so that the file keeps its size.
  u32 used = length < 0 ? 0 : (u32)length;
  if (used >= kFileSize) used = kFileSize - 1;
  memset(text + used, ' ', kFileSize - used);
  text[kFileSize - 1] = '\n';

  static bool is_directory_made = false;
  if (!is_directory_made) {
    is_directory_made = true;
    sys::File::CreateDirectory(u"sdmc:/sango");
  }
  sys::File file;
  file.Open(kPath);
  if (!file.IsOpen()) return;
  file.Write(text, kFileSize);
  file.Close();
}
} // namespace core
