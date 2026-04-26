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

#include "savedata/settings.h"

#include "menu/plugin_menu.h"

namespace savedata {

static const c8* TEXT_SPEED[] = {"Slow", "Normal", "Fast", "Instant"};
static const c8* TOGGLE_OFF_ON[] = {"Off", "On"};
static const c8* BATTLE_STYLE[] = {"Shift", "Set"};
static const c8* BUTTON_MODE[] = {"Normal", "L=A", "LR Disabled"};
static const c8* LANGUAGES[] = {"None",   "Japanese", "English",
                                "French", "Italian",  "German",
                                "---",    "Spanish",  "Korean"};
static const c8* BATTLE_BACKGROUNDS[] = {
    "Default", "Red",        "Blue",      "Pikachu", "Starters",
    "Eevee",   "Monochrome", "Stickers",  "Tatami",  "Floral Pattern",
    "Elegant", "Tall Grass", "Poke Ball", "Cockpit", "Carbon"};

void OnUpdateLanguage(void*) {
  Settings& settings = Settings::GetInstance();
  Core::GetInstance().GetLanguageId() = settings.language_id;  // 0x8C3ECC8
  WRITE(vu32, ADDRESS_LANGUAGE_ID, settings.language_id);
}

void Settings::LoadMenu(menu::PluginMenu& menu, void* args) {
  Settings& settings = GetInstance();

  menu.Add("Text Speed", &settings.core, 0, 2)
      .WithArray(TEXT_SPEED, SIZE(TEXT_SPEED))

      .Add("Show Battle Animations", &settings.core, 2, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))

      .Add("Battle Style", &settings.core, 3, 1)
      .WithArray(BATTLE_STYLE, SIZE(BATTLE_STYLE))

      .Add("Language ID (Press A)", &settings.core, 4, 4)
      .WithCallback(OnUpdateLanguage)
      .WithArray(LANGUAGES, SIZE(LANGUAGES))

      .Add("Battle Background", &settings.core, 8, 5)
      .WithArray(BATTLE_BACKGROUNDS, SIZE(BATTLE_BACKGROUNDS))

      .Add("Button Config Mode", &settings.core, 13, 2)
      .WithArray(BUTTON_MODE, SIZE(BUTTON_MODE))

      .Add("Prompt Save Before Net", &settings.core, 15, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))

      .Add("Enable SpotPass", &settings.core, 16, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON))

      .Add("Enable PSS", &settings.core, 17, 1)
      .WithArray(TOGGLE_OFF_ON, SIZE(TOGGLE_OFF_ON));
}

}  // namespace savedata