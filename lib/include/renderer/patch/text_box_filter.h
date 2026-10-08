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
 * @file text_box_filter.h
 * @brief Changes the text boxes of the 2D layouts (the menus).
 */

#pragma once

#include "common.h"
#include <type_traits>

namespace renderer {
struct TextBox;
}

namespace renderer {

/// The settings of renderer::TextBoxFilter.
struct TextBoxFilterSettings {
  bool is_enabled = false;
  Vec2 scale = Vec2(1, 1);
  Color8 top_color = Color8(0, 0, 0, 1);
  Color8 bottom_color = Color8(0, 0, 0, 1);
};
static_assert(std::is_standard_layout<TextBoxFilterSettings>::value,
              "TextBoxFilterSettings must have standard layout");

/// Changes the size and the colors of the texts of the menus.
struct TextBoxFilter : public TextBoxFilterSettings {
  MAKE_SINGLETON(TextBoxFilter)

  static void Initialize();

private:
  static u32 DrawHook(TextBox* text_box, u32 writer);
};

} // namespace renderer
