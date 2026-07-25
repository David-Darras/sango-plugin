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

#include "ui/main_application.h"

#include "game/event_manager.h"
#include "game/process_manager.h"
#include "system/device.h"
#include "system/graphics.h"
#include "system/sound.h"
#include "utils.h"
#include "feature/feature_device.h"
#include "feature/feature_process.h"
#include "ui/theme.h"

namespace ui {
MainApplication MainApplication::instance_ = MainApplication();

void MainApplication::DrawTop(Graphics& graphics) {
  if (!IsOpened()) return;

  painter_->DrawPageBackground(*this);
  painter_->DrawPageItems(*this);
}

void MainApplication::DrawSplashscreen(Graphics& graphics) {
  static s64 start = 0;
  if (start == 0) {
    Utils::GetElapsedTime(&start);
  }
  s64 now;
  Utils::GetElapsedTime(&now);
  s64 delta = now - start;
  u32 elapsed_sec = Utils::ConvertTimeToSeconds(&delta);
  if (elapsed_sec <= 5) {
    Color background(0,0,0,1);
    Graphics::FillScreen(background);
    Color text(1, 0, 0, 1);
    Graphics::SetTextScale(0.6, 0.6);
    Graphics::DrawText(5, 5 + 17 * 0, u"Pokémon Sango Kaizo", text);
    Graphics::DrawText(5, 5 + 17 * 1, u"Version 1.0", text);
    Graphics::DrawText(5, 5 + 17 * 2, u"Created By ZettaD", text);

    Graphics::DrawText(5, 5 + 17 * 4, u"Press [SELECT] to open in-game menu.",
                       text);
  }
}

void MainApplication::DrawBottom(Graphics& graphics) {
  DrawSplashscreen(graphics);

  if (!IsOpened() || !painter_->ShowBottom()) return;

  Graphics::FillScreen(theme_.background_color);

  if (GetSelectedEntry().GetType() == kTypeUnicode) {
    keyboard_.Draw();
  } else {
    numpad_.Draw();
  }

  uptr vtable = 0;
  c16 buffer[BUFFER_SIZE];
  auto& game_manager = game::ProcessManager::GetInstance();
  const char* process_name = game_manager.GetCurrentProcessName(vtable);
  Utils::Format(buffer, u"Process[%08X]=%s", vtable, process_name);
  Graphics::DrawText(5, 150, buffer, theme_.unselected_text_color);

  const char* event_name = game::EventManager::GetInstance().
      GetCurrentEventName(vtable);
  Utils::Format(buffer, u"Event[%08X]=%s", vtable, event_name);
  Graphics::DrawText(5, 170, buffer, theme_.unselected_text_color);

  Graphics::SetTextScale(0.5, 0.5);
  Utils::Format(buffer, u"Plugin by: %s", PLUGIN_CREATOR);
  Graphics::DrawText(5, 200, buffer, theme_.edited_text_color);
  Utils::Format(buffer, u"Ver: %s (Build: %s %s)", PLUGIN_VERSION, __DATE__,
                __TIME__);
  Graphics::DrawText(5, 216, buffer, theme_.edited_text_color);
}

void MainApplication::ForceClose() {
  Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
  is_opened_ = false;
  feature::DeviceState::GetInstance().use_redirection = is_opened_;
}

void MainApplication::Update(Controller& controller) {
  if (AreKeysReleased(controller)) {
    Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
    is_opened_ ^= 1;
    feature::DeviceState::GetInstance().use_redirection = is_opened_;
    return;
  }

  if (!IsOpened()) return;

  if (process_name_ != nullptr && !game::ProcessManager::GetInstance().
      IsCurrentProcess(process_name_)) {
    Close();
    return;
  }

  MenuContext& ctx = GetContext();
  PageItem& entry = GetSelectedEntry();

  if (painter_->ShowBottom()) {
    if (entry.GetType() == kTypeUnicode) {
      keyboard_.Update();
    } else {
      numpad_.Update();
    }
  }

  if (controller.IsKeyRepeated(Key::kRight)) {
    entry.Increment();
  } else if (controller.IsKeyRepeated(Key::kLeft)) {
    entry.Decrement();
  } else if (controller.IsKeyReleased(Key::kB)) {
    Close();
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

void MainApplication::Open(menu_callback_t load_menu, void* args) {
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

void MainApplication::Close() {
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

void MainApplication::Refresh() {
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

bool MainApplication::CheckProcess(const char* name) {
  if (!game::ProcessManager::GetInstance().IsCurrentProcess(name)) {
    while (contexts_count_ > 1) {
      Close();
    }
    Sound::PlaySoundEffect(theme_.error_sound);
    return true;
  }
  process_name_ = name;
  return false;
}

bool MainApplication::AreKeysReleased(Controller& controller) {
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

void MainAppPainter::DrawPageBackground(MainApplication& app) {
  if (app.no_background_) return;
  Graphics::FillScreen(app.theme_.background_color);
}

void MainAppPainter::DrawPageItems(MainApplication& app) {
  MainApplication::MenuContext& ctx = app.GetContext();

  Graphics::DrawText(5, 6 + ctx.cursor * MainApplication::kLineHeight,
                     u"\uE077", app.theme_.selected_text_color);

  c16 buffer[BUFFER_SIZE];
  for (u32 i = 0; i < ctx.display_count; i++) {
    app.entries_[i + ctx.offset].GetDisplayValue(buffer);
    Graphics::DrawText(25, 5 + i * MainApplication::kLineHeight, buffer,
                       ctx.cursor == i
                         ? app.theme_.selected_text_color
                         : app.theme_.unselected_text_color);
  }
}

void RetroAppPainter::DrawPageBackground(MainApplication& app) {
  Graphics::EnableScissor(301, 10, 90, 130);
  Graphics::FillScreen(1, 0.25, 0, 0.25);
  Graphics::DisableScissor();
}

void RetroAppPainter::DrawPageItems(MainApplication& app) {
  MainApplication::MenuContext& ctx = app.GetContext();

  Graphics::EnableScissor(303, 12, 86, 126);

  Graphics::FillScreen(0, 0, 0, 0.5);

  Color blue(1, 0.25, 0, 1);
  Color black(1, 1, 1, 1);

  Graphics::DrawText(391, 15 + 16 * ctx.cursor, u">",
                     blue);

  c16 buffer[BUFFER_SIZE];
  for (u32 i = 0; i < ctx.display_count; i++) {
    app.entries_[i + ctx.offset].GetDisplayValue(buffer);
    Graphics::DrawText(405, 15 + i * MainApplication::kLineHeight, buffer,
                       ctx.cursor == i
                         ? blue
                         : black);
  }

  Graphics::DisableScissor();
}
} // namespace ui