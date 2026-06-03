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
#include "menu/theme.h"

namespace menu {
PluginMenu PluginMenu::instance_ = PluginMenu();

void PluginMenu::DrawTop(Graphics& graphics) {
  if (!IsOpened()) return;

  LogMenu& log_menu = LogMenu::GetInstance();

  if (log_menu.IsEnabled()) {
    log_menu.Draw();
    return;
  }

  Controller& controller = Controller::GetInstance();
  MenuContext& ctx = GetContext();

  Color unselected_color = theme_.unselected_text_color;
  Color selected_color = theme_.selected_text_color;

  if (controller.IsKeyDown(Key::kLeft) || controller.IsKeyDown(Key::kRight) ||
      controller.IsKeyReleased(Key::kA)) {
    selected_color = theme_.edited_text_color;
  }

  // Draw the background.
  if (no_background_) {
    selected_color.a = 0.6;
    unselected_color.a = 0.6;
  } else {
    Graphics::FillScreen(theme_.background_color);
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

void PluginMenu::DrawBottom(Graphics& graphics) {
  if (!IsOpened()) return;

  // Draw the background.
  Graphics::FillScreen(theme_.background_color);

  // Draw the keyboard/numpad.
  if (GetSelectedEntry().GetType() == kTypeUnicode) {
    keyboard_.Draw();
  } else {
    numpad_.Draw();
  }

  // Show the current process + event of the game.
  c16 buffer[BUFFER_SIZE];
  const char* process_name = GameProcessManager::GetInstance().
      GetCurrentProcessName();
  Utils::Format(buffer, u"Process=%s", process_name);
  Graphics::DrawText(5, 150, buffer, theme_.unselected_text_color);

  const char* event_name = GameEventManager::GetInstance().
      GetCurrentEventName();
  Utils::Format(buffer, u"Event=%s", event_name);
  Graphics::DrawText(5, 170, buffer, theme_.unselected_text_color);

  Graphics::SetTextScale(0.5, 0.5);
  Utils::Format(buffer, u"Plugin by: %s", PLUGIN_CREATOR);
  Graphics::DrawText(5, 200, buffer, theme_.edited_text_color);
  Utils::Format(buffer, u"Ver: %s (Build: %s %s)", PLUGIN_VERSION, __DATE__,
                __TIME__);
  Graphics::DrawText(5, 216, buffer, theme_.edited_text_color);
}

void PluginMenu::Update(Controller& controller) {
  if (AreKeysReleased(controller)) {
    Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
    is_opened_ ^= 1;
    return;
  }

  if (!IsOpened()) return;

  if (process_name_ != nullptr && !GameProcessManager::GetInstance().
      IsCurrentProcess(process_name_)) {
    LeaveSubMenu();
    return;
  }

  LogMenu& log_menu = LogMenu::GetInstance();
  if (controller.IsKeyPressed(Key::kR)) {
    log_menu.Toggle();
  }

  MenuContext& ctx = GetContext();
  MenuEntry& entry = GetSelectedEntry();

  if (entry.GetType() == kTypeUnicode) {
    keyboard_.Update();
  } else {
    numpad_.Update();
  }

  if (controller.IsKeyRepeated(Key::kRight)) {
    entry.Increment();
  } else if (controller.IsKeyRepeated(Key::kLeft)) {
    entry.Decrement();
  } else if (controller.IsKeyReleased(Key::kB)) {
    LeaveSubMenu();
  } else if (controller.IsKeyReleased(Key::kA)) {
    Sound::PlaySoundEffect(theme_.confirm_sound);
    entry.Execute();
  } else if (controller.IsKeyReleased(Key::kX) || numpad_.IsButtonOkReleased()
             ||
             keyboard_.IsButtonOkReleased()) {
    Sound::PlaySoundEffect(theme_.confirm_sound);

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

  if (controller.IsKeyRepeated(Key::kLeft) || controller.IsKeyRepeated(
          Key::kRight) ||
      controller.IsKeyRepeated(Key::kDown) || controller.
      IsKeyReleased(Key::kUp)) {
    Sound::PlaySoundEffect(theme_.next_sound);
  }

  u8& offset = ctx.offset;
  u8& cursor = ctx.cursor;
  u8& display_count = ctx.display_count;

  if (controller.IsKeyRepeated(Key::kDown) && cursor < display_count - 1)
    cursor++;
  else if (controller.IsKeyRepeated(Key::kUp) && cursor > 0)
    cursor--;
  else if (controller.IsKeyRepeated(Key::kDown) &&
           (cursor + offset < entries_count_ - 1) &&
           (cursor == display_count - 1))
    offset++;
  else if (controller.IsKeyRepeated(Key::kUp) && (offset > 0) && (cursor == 0))
    offset--;
}

void PluginMenu::EnterSubMenu(menu_callback_t load_menu, void* args) {
  if (contexts_count_ >= kMaxContexts) return;

  contexts_[contexts_count_].Initialize(load_menu, args);
  contexts_count_++;

  entries_count_ = 0;
  no_background_ = 0;
  process_name_ = nullptr;
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
  no_background_ = 0;
  process_name_ = nullptr;
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

bool PluginMenu::CheckProcess(const char* name) {
  if (!GameProcessManager::GetInstance().IsCurrentProcess(name)) {
    while (contexts_count_ > 1) {
      LeaveSubMenu();
    }
    Sound::PlaySoundEffect(theme_.error_sound);
    return true;
  }
  process_name_ = name;
  return false;
}

bool PluginMenu::AreKeysReleased(Controller& controller) {
  if (theme_.keys[0] == 0) {
    return controller.IsKeyReleased(Key::kStart);
  }

  u32 key = 1 << (theme_.keys[0] - 1);
  bool res = controller.IsKeyReleased((Key)key);
  if (theme_.keys[1] == 0) {
    return res;
  }

  key = 1 << (theme_.keys[1] - 1);
  res &= controller.IsKeyReleased((Key)key);
  if (theme_.keys[2] == 0) {
    return res;
  }

  key = 1 << (theme_.keys[2] - 1);
  res &= controller.IsKeyReleased((Key)key);
  return res;
}
} // namespace menu