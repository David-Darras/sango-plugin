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

#include <type_traits>

#include "common.h"

namespace core {

struct ScriptLoaderSettings {
  bool dump_scripts = false;
  bool inject_scripts = false;
  bool log_activity = false;
  bool no_key_press = false;
  bool no_cutscene = false;
};
static_assert(std::is_standard_layout<ScriptLoaderSettings>::value,
              "ScriptLoaderSettings must have standard layout");

/// Dumps the field scripts the game loads to the SD card and loads edited
/// ones back in their place; C++ scripts (script::NativeScript) take
/// precedence over both.
class ScriptLoader : public ScriptLoaderSettings {
  MAKE_SINGLETON(ScriptLoader)

public:
  static constexpr u32 kMaxScriptBytes = 64 * 1024;
  static constexpr u32 kDumpHistorySize = 128;

  u32 dumped_count = 0;
  u32 injected_count = 0;

  static void Initialize();
  static void PatchLoad();
  static void LoadHook(void* self, const void* buffer, u32 size,
                       const c8* amxname);

private:
  static u32 ComputeId(const void* data, u32 size);
  static bool IsValidHeader(const void* buffer, u32 capacity);
  void Dump(u32 id, const void* buffer, u32 size);
  bool MarkSeen(u32 id);
  void AppendIndexEntry(u32 id, u32 size);
  const void* LoadOverride(u32 id, u32* out_size);
  void ReportInjected(u32 id, u32 size);
  static u8* OverrideBuffer();

  u32 dump_history_[kDumpHistorySize] = {};
  u32 dump_history_next_ = 0;
  s64 index_offset_ = 0;
};
} // namespace core
