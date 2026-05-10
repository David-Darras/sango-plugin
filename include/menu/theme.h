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
  Color background_color;
  Color unselected_text_color;
  Color selected_text_color;
  Color edited_text_color;

  u16 open_sound;
  u16 close_sound;
  u16 confirm_sound;
  u16 next_sound;
  u16 error_sound;

  u8 keys[3];

  static void LoadMenu(PluginMenu& menu, void* args);

  static Theme& GetInstance() {
    return instance;
  }

private:
  Theme();
  static Theme instance;
};
} // namespace menu

#endif //CTR_PLUGIN_TOOLKIT_THEME_H