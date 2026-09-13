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
#include "system/constant/language.h"

namespace sys {
class Device;
class Graphics;
class FontManager;

class Core {
  SINGLETON(Core)
public:
  STATIC_INLINE Core& GetInstance() { return *(Core*)core::address::kCore; }

  INLINE Device& GetDevice() const { return *device_; }
  INLINE Graphics& GetGraphics() const { return *graphics_; }
  INLINE Language& GetLanguage() const { return *language; }
  INLINE FontManager& GetFontManager() const { return *font_manager_; }

private:
  Device* device_;
  Graphics* graphics_;
  void* _0[21];
  Language* language;
  FontManager* font_manager_;
};

} // namespace sys
