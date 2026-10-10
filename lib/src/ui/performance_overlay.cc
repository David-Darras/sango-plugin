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
 * @file performance_overlay.cc
 * @brief Shows the frames per second and the free memory on the top screen.
 *
 * The declarations are in ui/performance_overlay.h.
 */

#include "ui/performance_overlay.h"

#include <3ds.h>

#include "core/game_file.h"
#include "core/utils.h"
#include "system/native/graphics.h"
#include "ui/main_application.h"

namespace ui {
namespace {
// The ticks of the system clock in one millisecond.
constexpr f32 kTicksPerMs = SYSCLOCK_ARM11 / 1000.0f;
// The frames between two reads of the free memory.
constexpr u32 kMemoryFrames = 30;
// The weight of the new frame in the moving average.
constexpr f32 kAverageWeight = 0.1f;
// A frame that takes more time is a pause (for example the HOME Menu).
constexpr f32 kMaxFrameMs = 500.0f;
} // namespace

void PerformanceOverlay::DrawTop() {
  PerformanceOverlay& overlay = GetInstance();
  const u64 tick = svcGetSystemTick();
  if (overlay.last_tick_ != 0) {
    const f32 ms = (f32)(tick - overlay.last_tick_) / kTicksPerMs;
    if (ms < kMaxFrameMs) {
      overlay.frame_ms_ = overlay.frame_ms_ == 0
                              ? ms
                              : overlay.frame_ms_ +
                                    (ms - overlay.frame_ms_) * kAverageWeight;
    }
  }
  overlay.last_tick_ = tick;
  if (!overlay.is_enabled || MainApplication::GetInstance().IsOpened()) {
    return;
  }

  if (overlay.counter_ == 0) {
    overlay.linear_kb_ = osGetMemRegionFree(MEMREGION_APPLICATION) / 1024;
    overlay.heap_kb_[0] = core::GameFile::GetFreeHeapMemory(0) / 1024;
    overlay.heap_kb_[1] = core::GameFile::GetFreeHeapMemory(1) / 1024;
    overlay.counter_ = kMemoryFrames;
  }
  overlay.counter_--;

  // A dark box at the bottom-left corner, then two lines. The texts of the
  // top screen use a space of 512 x 256.
  sys::Graphics::DrawRect(0, 214, 230, 26, Color(0, 0, 0, 0.6f));
  const f32 fps = overlay.frame_ms_ > 0 ? 1000.0f / overlay.frame_ms_ : 0;
  c16 text[96];
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  core::Utils::Format(text, u"%.1f FPS   %.1f ms", fps, overlay.frame_ms_);
  sys::Graphics::DrawText(4, 229, text,
                          fps < 25.0f ? Color(1, 0.4f, 0.3f, 1)
                                      : Color(1, 1, 1, 1));
  core::Utils::Format(text, u"Linear %lu KB   Heap %lu KB   Device %lu KB",
                      overlay.linear_kb_, overlay.heap_kb_[0],
                      overlay.heap_kb_[1]);
  sys::Graphics::DrawText(4, 242, text, Color(1, 1, 1, 1));
}
} // namespace ui
