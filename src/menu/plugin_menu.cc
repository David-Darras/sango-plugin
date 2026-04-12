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

#include "menu/plugin_menu.h"

#include "core/game_event_manager.h"
#include "core/game_process_manager.h"
#include "menu/log_menu.h"
#include "system/device.h"
#include "system/graphics.h"
#include "system/sound.h"
#include "utils.h"

namespace menu {

PluginMenu PluginMenu::instance_ = PluginMenu();

void PluginMenu::DrawTop() {
  LogMenu& log_menu = LogMenu::GetInstance();
  if (log_menu.IsEnabled()) {
    log_menu.Draw();
    return;
  }

  Controller& ctrl = Controller::GetInstance();
  MenuContext& ctx = GetContext();

  Color unselected_color{1, 1, 1, 1};
  Color selected_color{0, 1, 0, 1};

  if (ctrl.IsKeyDown(Key::kLeft) || ctrl.IsKeyDown(Key::kRight) ||
      ctrl.IsKeyReleased(Key::kA)) {
    selected_color = {1, 0, 0, 1};
  }

  // Draw the background.
  if (no_background) {
    selected_color.a = 0.6;
    unselected_color.a = 0.6;
  } else {
    Graphics::FillScreen(0, 0, 0, 0.75);
  }

  // Draw the cursor.
  Graphics::DrawText(5, 6 + ctx.cursor * kLineHeight, u"\uE077",
                     selected_color);

  // Draw the entries.
  c16 buffer[BUFFER_SIZE];
  for (u32 i = 0; i < ctx.display_count; i++) {
    entries_[i + ctx.offset].GetDisplayValue(buffer);
    Graphics::DrawText(25, 5 + i * kLineHeight, buffer,
                       ctx.cursor == i ? selected_color : unselected_color);
  }
}

void PluginMenu::DrawBottom() {
  // Draw the background.
  Graphics::FillScreen(0, 0, 0, 0.75);

  // Draw the keyboard/numpad.
  if (GetSelectedEntry().GetType() == kTypeUnicode) {
    keyboard_.Draw();
  } else {
    numpad_.Draw();
  }

  // Show the current process + event of the game.
  c16 buffer[BUFFER_SIZE];
  uptr baseAddr =
      (uptr)&GameProcessManager::GetInstance().GetMainHandle().GetProcess();
  void* vtable = (void*)*(u32*)baseAddr;
  Utils::Format(buffer, u"Process=%s", Utils::GetClassNameFromVTable(vtable));
  Graphics::DrawText(5, 150, buffer);

  baseAddr = (uptr)&GameEventManager::GetInstance().GetGameEvent();
  vtable = (void*)*(u32*)baseAddr;
  Utils::Format(buffer, u"Event=%s", Utils::GetClassNameFromVTable(vtable));
  Graphics::DrawText(5, 170, buffer);

  Color yellow{1, 1, 0, 1};
  Graphics::SetTextScale(0.5, 0.5);
  Utils::Format(buffer, u"Plugin by: %s", PLUGIN_CREATOR);
  Graphics::DrawText(5, 200, buffer, yellow);
  Utils::Format(buffer, u"Ver: %s (Build: %s %s)", PLUGIN_VERSION, __DATE__,
                __TIME__);
  Graphics::DrawText(5, 216, buffer, yellow);
}

void PluginMenu::Update() {
  Controller& ctrl = Controller::GetInstance();

  if (ctrl.IsKeyReleased(Key::kStart)) {
    Sound::PlaySoundEffect(IsOpened() ? 8 : 7);
    is_opened_ ^= 1;
    return;
  }

  if (!IsOpened()) return;

  LogMenu& log_menu = LogMenu::GetInstance();
  if (ctrl.IsKeyPressed(Key::kR)) {
    log_menu.Toggle();
  }

  if (ctrl.IsKeyPressed(Key::kL)) {
    GameProcessManager& instance = GameProcessManager::GetInstance();
    log_menu.Add(u"process manager=%08X", &instance);
    log_menu.Add(u"main handle=%08X", &instance.GetMainHandle());
    log_menu.Add(u"current process=%08X",
                 &instance.GetMainHandle().GetProcess());
  }

  MenuContext& ctx = GetContext();
  MenuEntry& entry = GetSelectedEntry();

  if (entry.GetType() == kTypeUnicode) {
    keyboard_.Update();
  } else {
    numpad_.Update();
  }

  if (ctrl.IsKeyRepeated(Key::kRight)) {
    entry.Increment();
  } else if (ctrl.IsKeyRepeated(Key::kLeft)) {
    entry.Decrement();
  } else if (ctrl.IsKeyReleased(Key::kB)) {
    LeaveSubMenu();
  } else if (ctrl.IsKeyReleased(Key::kA)) {
    entry.Execute();
  } else if (ctrl.IsKeyReleased(Key::kX) || numpad_.IsButtonOkReleased() ||
             keyboard_.IsButtonOkReleased()) {
    Sound::PlaySoundEffect(0);

    switch (entry.GetType()) {
      case kTypeUnicode:
        entry.Edit(keyboard_.GetInput());
        break;
      default:
        u32 input = numpad_.GetInput();
        entry.Edit(&input);
        break;
    }
  }

  if (ctrl.IsKeyRepeated(Key::kLeft) || ctrl.IsKeyRepeated(Key::kRight) ||
      ctrl.IsKeyRepeated(Key::kDown) || ctrl.IsKeyReleased(Key::kUp)) {
    Sound::PlaySoundEffect(4);
  }

  u8& offset = ctx.offset;
  u8& cursor = ctx.cursor;
  u8& display_count = ctx.display_count;

  if (ctrl.IsKeyRepeated(Key::kDown) && cursor < display_count - 1)
    cursor++;
  else if (ctrl.IsKeyRepeated(Key::kUp) && cursor > 0)
    cursor--;
  else if (ctrl.IsKeyRepeated(Key::kDown) &&
           (cursor + offset < entries_count_ - 1) &&
           (cursor == display_count - 1))
    offset++;
  else if (ctrl.IsKeyRepeated(Key::kUp) && (offset > 0) && (cursor == 0))
    offset--;
}

void PluginMenu::EnterSubMenu(menu_callback_t load_menu, void* args) {
  if (contexts_count_ >= kMaxContexts) return;

  contexts_[contexts_count_].Initialize(load_menu, args);
  contexts_count_++;

  entries_count_ = 0;
  no_background = 0;
  load_menu(*this, args);

  MenuContext& ctx = GetContext();
  ctx.display_count =
      entries_count_ > kMaxDisplayCount ? kMaxDisplayCount : entries_count_;
}

void PluginMenu::LeaveSubMenu() {
  if (contexts_count_ <= 1) return;

  contexts_count_--;

  MenuContext& ctx = GetContext();

  entries_count_ = 0;
  no_background = 0;
  ctx.load_menu(*this, ctx.args);

  ctx.display_count =
      entries_count_ > kMaxDisplayCount ? kMaxDisplayCount : entries_count_;
}

void PluginMenu::Refresh() {
  MenuContext& ctx = GetContext();
  entries_count_ = 0;
  ctx.load_menu(*this, ctx.args);
  ctx.display_count =
      (entries_count_ > kMaxDisplayCount) ? kMaxDisplayCount : entries_count_;
  if (ctx.cursor + ctx.offset >= entries_count_) {
    ctx.cursor = 0;
    ctx.offset = 0;
  }
}

}  // namespace menu