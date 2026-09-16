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

#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "core/patch/device_patch.h"
#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "ui/theme.h"

namespace ui {
MainApplication MainApplication::instance_ = MainApplication();

void MainApplication::DrawTop(sys::Graphics& graphics) {
  if (!IsOpened()) return;

  painter_->DrawPageBackground(*this);
  sys::Graphics::SetTextScale(1.0f, 1.0f);
  sys::Graphics::DrawText(390, 220, u"『さんご』",
                     Theme::GetInstance().selected_text_color);
  painter_->DrawPageItems(*this);
}

#include <cxxabi.h>
#include <cstdlib>
#include <cstring>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

void MainApplication::DrawBottom(sys::Graphics& graphics) {
  painter_->DrawBottomOverlay(graphics);

  if (!IsOpened() || !painter_->ShowBottom()) return;

  sys::Graphics::FillScreen(theme_.background_color);
  sys::Graphics::DrawRectStroke(0, 0, 320, 240, 1,
                           theme_.selected_text_color);

  if (GetSelectedEntry().GetType() == kTypeUnicode) {
    keyboard_.Draw();
  } else {
    numpad_.Draw();
  }

  uptr vtable = 0;
  c16 buffer[BUFFER_SIZE];
  auto& game_manager = core::ProcessManager::GetInstance();
  const char* process_name = core::Utils::Unmangle(
      game_manager.GetCurrentProcessName(vtable));

  core::Utils::Format(buffer, u"Process[%s]", process_name);
  sys::Graphics::DrawText(5, 150, buffer, theme_.unselected_text_color);

  const char* event_name = core::Utils::Unmangle(
      core::EventManager::GetInstance().GetCurrentEventName(vtable));
  core::Utils::Format(buffer, u"Event[%s]", event_name);
  sys::Graphics::DrawText(5, 170, buffer, theme_.unselected_text_color);

  sys::Graphics::SetTextScale(0.5, 0.5);
  core::Utils::Format(buffer, u"Sango Plugin | Created by %s",
                PLUGIN_CREATOR);
  sys::Graphics::DrawText(5, 200, buffer, theme_.selected_text_color);
  core::Utils::Format(buffer, u"Build: %s %s", PLUGIN_VERSION, __DATE__,
                __TIME__);
  sys::Graphics::DrawText(5, 216, buffer, theme_.selected_text_color);
}

void MainApplication::ForceClose() {
  sys::Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
  is_opened_ = false;
  core::DevicePatch::GetInstance().use_redirection = is_opened_;
}

void MainApplication::Update(sys::Controller& controller) {
  if (AreKeysReleased(controller)) {
    sys::Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
    is_opened_ ^= 1;
    core::DevicePatch::GetInstance().use_redirection = is_opened_;
    return;
  }

  if (!IsOpened()) return;

  if (process_vtable_ != 0 && !core::ProcessManager::GetInstance().
      IsCurrentProcess(process_vtable_)) {
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
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
    entry.Execute(*this);
  } else if (controller.IsKeyReleased(Key::kX) || numpad_.IsButtonOkReleased()
             ||
             keyboard_.IsButtonOkReleased()) {
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);

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
      IsKeyRepeated(Key::kUp)) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
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
  process_vtable_ = 0;
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
  process_vtable_ = 0;
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

bool MainApplication::CheckProcess(uptr vtable) {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(vtable)) {
    while (contexts_count_ > 1) {
      Close();
    }
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return true;
  }
  process_vtable_ = vtable;
  return false;
}

bool MainApplication::AreKeysReleased(sys::Controller& controller) {
  if (theme_.keys[0] == 0) {
    return controller.IsKeyReleased(Key::kStart);
  }

  bool res = true;
  for (u32 i = 0; i < SIZE(theme_.keys) && theme_.keys[i] != 0; i++) {
    u32 key = 1 << (theme_.keys[i] - 1);
    res &= controller.IsKeyReleased((Key)key);
  }
  return res;
}

u32 Painter::GetCursor(MainApplication& app) { return app.GetContext().cursor; }

u32 Painter::GetOffset(MainApplication& app) { return app.GetContext().offset; }

u32 Painter::GetDisplayCount(MainApplication& app) {
  return app.GetContext().display_count;
}

PageItem& Painter::GetEntry(MainApplication& app, u32 index) {
  return app.entries_[index];
}

void MainAppPainter::DrawPageBackground(MainApplication& app) {
  if (!app.no_background_) {
    sys::Graphics::FillScreen(app.theme_.background_color);
  }
  sys::Graphics::DrawRectStroke(0, 0, 400, 240, 1,
                           Theme::GetInstance().selected_text_color);
}

void MainAppPainter::DrawPageItems(MainApplication& app) {
  MainApplication::MenuContext& ctx = app.GetContext();

  sys::Graphics::SetTextScale(0.6, 0.6);
  sys::Graphics::DrawText(5, 6 + ctx.cursor * MainApplication::kLineHeight,
                     u"\uE077", app.theme_.selected_text_color);

  c16 buffer[BUFFER_SIZE];
  for (u32 i = 0; i < ctx.display_count; i++) {
    app.entries_[i + ctx.offset].GetDisplayValue(buffer);
    sys::Graphics::DrawText(25, 5 + i * MainApplication::kLineHeight, buffer,
                       ctx.cursor == i
                         ? app.theme_.selected_text_color
                         : app.theme_.unselected_text_color);
  }
}

} // namespace ui
