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
 * @file core.h
 * @brief The object of the game that owns the devices, the graphics and the fonts.
 */

#pragma once

#include "common.h"
#include "system/constant/language.h"

namespace sys {
class Device;
class Graphics;
class FontManager;

/// The object of the game that owns the devices, the graphics and the fonts.
class Core {
  SINGLETON(Core)
public:
  STATIC_INLINE Core& GetInstance() { return *(Core*)core::address::kCore; }

  /// Returns the input devices.
  INLINE Device& GetDevice() const { return *device_; }
  /// Returns the graphics system.
  INLINE Graphics& GetGraphics() const { return *graphics_; }
  /// Returns the language of the game.
  INLINE Language& GetLanguage() const { return *language; }
  /// Returns the fonts.
  INLINE FontManager& GetFontManager() const { return *font_manager_; }
  /// Restarts the game (soft reset).
  INLINE void ForceReset() { reset = true; }

private:
  Device* device_;
  Graphics* graphics_;
  void* _0[21];
  Language* language;
  FontManager* font_manager_;
  u32 _1[4];
  bool _2;
  bool reset;
  bool _3;
  bool _4;
};
} // namespace sys