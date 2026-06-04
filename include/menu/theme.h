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

#ifndef SANGO_PLUGIN_MENU_THEME_H
#define SANGO_PLUGIN_MENU_THEME_H

#include "common.h"

namespace menu {
struct Theme {
  MAKE_SINGLETON(Theme)
  Color background_color = Color{0, 0, 0, 0.75f};
  Color unselected_text_color = Color{1, 1, 1, 1};
  Color selected_text_color = Color{0, 1, 0, 1};
  Color edited_text_color = Color{1, 0, 0, 1};

  u16 open_sound = 7;
  u16 close_sound = 8;
  u16 confirm_sound = 0;
  u16 next_sound = 4;
  u16 error_sound = 21;

  u8 keys[3];
};
} // namespace menu

#endif // SANGO_PLUGIN_MENU_THEME_H