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
 * @file problems.h
 * @brief The problems of the ROM hack that the library finds at the start.
 *
 * The functions of the library that add data (AddMove(), Register()...)
 * report their problems here: an id that is not free, a list that is full...
 * After the start, the library checks the ROM hack again (CheckHack()).
 *
 * When there is a problem, the top screen shows the list a few seconds after
 * the start. The player closes it with A or B. The log of the plugin
 * (Plugin > Log) also shows each problem.
 *
 * @see docs/getting-started/05-troubleshooting.md
 */

#pragma once

#include "common.h"

namespace core {

/// The problems of the ROM hack that the library finds at the start.
class Problems {
  MAKE_SINGLETON(Problems)

public:
  /// The number of problems that the list keeps. The next ones go to the log
  /// only.
  static constexpr u32 kMaxProblems = 10;
  /// The length of a problem, with the end of the text.
  static constexpr u32 kMaxLength = 64;

  /**
   * @brief Adds a problem. It also goes to the log of the plugin.
   * @param format The text, with the format of printf (%d, %u, %s...).
   */
  static void Add(const c16* format, ...);

  /// Returns the number of problems, also the problems that the list does not
  /// keep.
  static u32 GetCount() { return GetInstance().count_; }

  /// Returns a problem of the list.
  static const c16* Get(u32 index) {
    return GetInstance().texts_[index < kMaxProblems ? index : 0];
  }

  /// Checks the ROM hack after its settings: the ids of the scripts, the
  /// scripts of the characters, the icon files... plugin::Start() calls it.
  static void CheckHack();

  /// Draws the list of the problems on the top screen, after the splash
  /// screen. The player closes it with A or B. Call it one time for each
  /// frame.
  static void DrawTop();

private:
  c16 texts_[kMaxProblems][kMaxLength] = {};
  u32 count_ = 0;
  bool is_closed_ = false;
  u32 frame_ = 0;
};

} // namespace core
