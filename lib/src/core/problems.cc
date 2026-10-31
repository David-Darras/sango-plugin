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
 * @file problems.cc
 * @brief The problems of the ROM hack that the library finds at the start.
 *
 * The declarations are in core/problems.h.
 */

#include "core/problems.h"
#include "core/data_folder.h"

#include <cstdarg>

#include "battle/patch/game_extension.h"
#include "battle/patch/item_extension.h"
#include "overworld/patch/map_character.h"
#include "script/patch/native_script.h"
#include "system/address.h"
#include "system/native/controller.h"
#include "system/native/file.h"
#include "system/native/graphics.h"
#include "ui/log.h"
#include "ui/splash.h"
#include "ui/theme.h"

namespace core {
namespace {
// The list waits for the end of the splash screen.
constexpr u32 kFirstFrame = ui::Splash::kFrames;
// The number of problems that the screen shows.
constexpr u32 kShownProblems = 7;
// The number of different models for the added characters. The other half of
// the 33 models of a region is for the characters of the game.
constexpr u32 kMaxCharacterModels = 16;

bool IsFileOnSdCard(const c16* path) {
  sys::File file;
  file.Open(path, sys::File::kRead);
  const bool is_open = file.IsOpen();
  file.Close();
  return is_open;
}

bool IsCustomScript(ScriptId id) {
  const u32 value = static_cast<u32>(id);
  return value >= static_cast<u32>(ScriptId::kFirstCustom) &&
         value <= static_cast<u32>(ScriptId::kLastCustom);
}

void CheckScripts() {
  for (u32 i = 0; i < script::NativeScript::GetCount(); i++) {
    const ScriptId id = script::NativeScript::GetId(i);
    if (!IsCustomScript(id)) {
      Problems::Add(u"script %u: use an id from 30500 to 59999",
                    static_cast<u32>(id));
    }
  }
}

void CheckCharacters() {
  // The game loads the models of all the added characters with the models of
  // the characters of the region: 33 models at most.
  ModelId models[overworld::MapCharacter::kMaxRequests];
  u32 model_count = 0;
  for (u32 i = 0; i < overworld::MapCharacter::GetCount(); i++) {
    const ModelId model = overworld::MapCharacter::GetRequest(i).model_id;
    bool is_new = true;
    for (u32 j = 0; j < model_count; j++) {
      if (models[j] == model) is_new = false;
    }
    if (is_new) models[model_count++] = model;
  }
  if (model_count > kMaxCharacterModels) {
    Problems::Add(u"characters: %u models, use %u at most", model_count,
                  kMaxCharacterModels);
  }

  for (u32 i = 0; i < overworld::MapCharacter::GetCount(); i++) {
    const auto& request = overworld::MapCharacter::GetRequest(i);
    if (request.trainer_id != TrainerId::kNone && request.sight > 10) {
      Problems::Add(u"character %u: the trainer sees %u tiles, 10 at most",
                    i + 1, request.sight);
    }
    // A script of the game is fine. A free id must have a C++ script.
    if (IsCustomScript(request.script_id) &&
        !script::NativeScript::IsRegistered(request.script_id)) {
      Problems::Add(u"character %u: script %u is not registered", i + 1,
                    static_cast<u32>(request.script_id));
    }
  }
}

void CheckTypeIcons() {
  for (u32 id = 0; id < battle::GameExtension::kMaxTypes; id++) {
    const battle::TypeSpec* spec = battle::GameExtension::FindType(id);
    if (spec == nullptr || spec->name == nullptr) continue;
    c16 path[96];
    if (battle::GameExtension::GetTypeIconPath(*spec, path, SIZE(path)) &&
        !IsFileOnSdCard(path)) {
      Problems::Add(u"type %u: no icon in " SANGO_DATA_PATH(u"types"), id);
    }
  }
}

void CheckItemIcons() {
  for (u32 i = 0; i < battle::ItemExtension::GetCount(); i++) {
    const battle::ItemSpec& spec = battle::ItemExtension::GetItem(i);
    // An item without its own icon can use the icon of an item of the game.
    if (spec.icon_item != ItemId::kNone) continue;
    c16 path[96];
    if (battle::ItemExtension::GetIconPath(spec, path, SIZE(path)) &&
        !IsFileOnSdCard(path)) {
      Problems::Add(u"item %u: no icon in " SANGO_DATA_PATH(u"items"),
                    static_cast<u32>(spec.id));
    }
  }
}
} // namespace

void Problems::Add(const c16* format, ...) {
  if (format == nullptr) return;
  auto& self = GetInstance();

  c16 text[kMaxLength];
  va_list args;
  va_start(args, format);
  ((void (*)(c16*, u32, const c16*, va_list))sys::address::kStdVswprintf)(
      text, kMaxLength, format, args);
  va_end(args);
  text[kMaxLength - 1] = u'\0';

  if (self.count_ < kMaxProblems) {
    for (u32 i = 0; i < kMaxLength; i++) self.texts_[self.count_][i] = text[i];
  }
  self.count_++;

  // The log formats its text again: a % of the text becomes a ?.
  for (u32 i = 0; text[i] != u'\0'; i++) {
    if (text[i] == u'%') text[i] = u'?';
  }
  ui::Log::Print(text);
}

void Problems::CheckHack() {
  CheckScripts();
  CheckCharacters();
  CheckTypeIcons();
  CheckItemIcons();
}

void Problems::DrawTop() {
  auto& self = GetInstance();
  if (self.count_ == 0 || self.is_closed_) return;
  if (self.frame_ < kFirstFrame) {
    self.frame_++;
    return;
  }

  auto& controller = sys::Controller::GetInstance();
  if (controller.IsKeyPressed(Key::kA) || controller.IsKeyPressed(Key::kB)) {
    self.is_closed_ = true;
    return;
  }

  const ui::Theme& theme = ui::Theme::GetInstance();
  sys::Graphics::FillScreen(Color(0, 0, 0, 0.8f));

  // The texts of the top screen use a space of 512 x 256.
  c16 line[96];
  sys::Graphics::SetTextScale(0.7f, 0.7f);
  core::Utils::Format(line, u"The ROM hack has %u problem(s)", self.count_);
  sys::Graphics::DrawTextWithShadow(24, 20, line, Color(1, 0.4f, 0.4f, 1));

  sys::Graphics::SetTextScale(0.5f, 0.5f);
  const u32 shown = self.count_ < kShownProblems ? self.count_ : kShownProblems;
  for (u32 i = 0; i < shown; i++) {
    sys::Graphics::DrawTextWithShadow(24, 60 + 22 * i, self.texts_[i],
                                      theme.unselected_text_color);
  }
  if (self.count_ > shown) {
    core::Utils::Format(line, u"... and %u more: see Plugin > Log",
                        self.count_ - shown);
    sys::Graphics::DrawTextWithShadow(24, 60 + 22 * shown, line,
                                      theme.unselected_text_color);
  }
  sys::Graphics::DrawTextWithShadow(24, 225, u"Press A or B to close",
                                    theme.selected_text_color);
}

} // namespace core
