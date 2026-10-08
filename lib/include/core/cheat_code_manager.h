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
 * @file cheat_code_manager.h
 * @brief The list of the cheat codes of the plugin.
 */

#pragma once

#include "core/cheat_code.h"
#include "core/constant/cheat_code_id.h"

namespace core {

/**
 * @brief Keeps the cheat codes and runs them at each frame.
 *
 * A menu entry can enable a cheat code: `app.Add("Repel", CheatCodeId::kNoEncounter)`.
 */
class CheatCodeManager {
  MAKE_SINGLETON(CheatCodeManager)

public:
  void Add(CheatCodeId id, cheat_code_callback_t on_enable,
           cheat_code_callback_t on_disable,
           bool do_each_frame);

  /// Returns the cheat code with this id, or null.
  CheatCode* Get(CheatCodeId id);
  /// Runs the cheat codes that run at each frame. plugin::UpdateFrame() calls it.
  void Update() const;

  /// Adds a cheat code. A feature calls it in its Initialize().
  static void Initialize(CheatCodeId id, cheat_code_callback_t on_enable,
                         cheat_code_callback_t on_disable,
                         bool do_each_frame);

private:
  static constexpr u32 kMaxCheatCodes = (u32)CheatCodeId::kMax;

  CheatCode cheat_codes_[kMaxCheatCodes];
  u32 count_ = 0;
};

} // namespace core

