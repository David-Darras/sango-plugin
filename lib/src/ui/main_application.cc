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
 * @file main_application.cc
 * @brief The menu of the plugin. See ui/main_application.h.
 *
 * The quick access, the search and the radial menu are in
 * menu_shortcuts.cc. The settings file is in menu_settings.cc.
 */

#include "ui/main_application.h"

#include <cstring>

#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "core/patch/device_patch.h"
#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "ui/theme.h"
#include "ui/menu_layout.h"
#include "ui/widget/image.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

namespace ui {
using namespace layout;

namespace {

// Left / Right change a number 10 times faster after this number of frames.
constexpr u32 kFastStepFrames = 45;

// The name, the version and the author of the plugin, and the date of the
// build. The first page shows it.
const c8* const kCredit = PLUGIN_NAME " " PLUGIN_VERSION " by " PLUGIN_CREATOR
                          "  |  Build " __DATE__ " " __TIME__;

// Each DrawText() adds many commands to the command list of the GPU, and
// the list has a fixed size: about 100 texts in one frame make the game
// crash (see sys::Graphics::kTextLimit). So the menu draws few texts: one
// text for each row of keys. Then the shadows of the texts fit.

// The background images of the menu (see Theme::background_image).
Image& GetTopImage() {
  static Image image(u"sdmc:/sango/menu_top.tga");
  return image;
}

Image& GetBottomImage() {
  static Image image(u"sdmc:/sango/menu_bottom.tga");
  return image;
}

// Draws a text (UTF-8) at the position, with a maximum of `max_length`
// characters. Returns the number of characters that it drew.
u32 DrawPart(s32 x, s32 y, const c8* text, u32 max_length, Color color) {
  c8 part[kWrapLength + 1];
  u32 length = strlen(text);
  if (max_length > kWrapLength) max_length = kWrapLength;
  if (length > max_length) {
    // Cut the text after the last space, when there is one.
    length = max_length;
    for (u32 i = max_length; i > max_length / 2; i--) {
      if (text[i] == ' ') {
        length = i;
        break;
      }
    }
  }
  memcpy(part, text, length);
  part[length] = '\0';
  c16 buffer[kWrapLength + 1];
  core::Utils::Format(buffer, u"%s", part);
  DrawMenuText(x, y, buffer, color);
  return length;
}

// Draws a text on two lines at most.
void DrawWrapped(s32 x, s32 y, const c8* text, Color color) {
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  const u32 first = DrawPart(x, y, text, kWrapLength, color);
  const c8* rest = text + first;
  while (*rest == ' ') rest++;
  if (*rest != '\0') {
    sys::Graphics::SetTextScale(0.45f, 0.45f);
    DrawPart(x, y + 12, rest, kWrapLength, color);
  }
}

// The help line of an entry without a description.
const c8* GetHint(u8 type, bool has_texts) {
  switch (type) {
    case kTypeMenu:
      return "Press A or touch the button to open the page.";
    case kTypeIdle:
      return "Press A or touch the button to run the action.";
    case kTypeBoolean:
    case kTypeCheatCode:
      return "Press A, Left / Right, or touch On / Off.";
    case kTypeUnicode:
      return "Type the text on the keyboard, then touch OK.";
    case kTypeSeparator:
      return "";
    default:
      return has_texts ? "Press Left / Right, or touch a value."
                       : "Press Left / Right, type a number, or touch a step.";
  }
}

// Returns the number of characters of a UTF-16 text.
u32 GetLength(const c16* text) {
  u32 length = 0;
  while (text[length] != 0) length++;
  return length;
}

} // namespace

MainApplication MainApplication::instance_ = MainApplication();

void MainApplication::DrawTop(sys::Graphics& graphics) {
  if (!IsOpened()) return;

  // Read the background images (one file in each frame), and the icons.
  if (theme_.background_image && !GetTopImage().Prepare()) {
    GetBottomImage().Prepare();
  }
  IconPool::GetInstance().Prepare();

  painter_->DrawPageBackground(*this);
//   sys::Graphics::SetTextScale(1.0f, 1.0f);
// #ifdef GAME_ORAS
//   static const c16* kPluginTitle = u"『さんご』";
// #endif
// #ifdef GAME_XY
//   static const c16* kPluginTitle = u"『くじら』";
// #endif
//   sys::Graphics::DrawText(390, 220, kPluginTitle,
//                           Theme::GetInstance().selected_text_color);
  painter_->DrawPageItems(*this);
}

void MainApplication::InitializeTouch() {
  // The navigation bar: five large buttons at the bottom edge.
  for (u32 i = 0; i < kNavCount; i++) {
    touch_[kTouchSectionUp + i].Initialize(4 + i * 63, kNavY, 59, kNavHeight);
  }
  // Undo: the top-right corner, easy to reach with the stylus.
  touch_[kTouchUndo].Initialize(262, 2, 54, 24);
  touch_[kTouchOff].Initialize(10, kEditorY + 10, 145, 50);
  touch_[kTouchOn].Initialize(165, kEditorY + 10, 145, 50);
  touch_[kTouchRun].Initialize(10, kEditorY + 10, 300, 50);
  // The step buttons, on the right of the keys of the numpad: -1 and +1 on
  // the first row, -10 and +10 on the second row.
  const s32 step_x = 10 + Numpad::kKeysWidth + 6;
  const s32 step_width = (300 - Numpad::kKeysWidth - 6 - 2) / 2;
  const s32 step_y = kEditorY + 24;
  touch_[kTouchMinus1].Initialize(step_x, step_y, step_width, 25);
  touch_[kTouchPlus1].Initialize(step_x + step_width + 2, step_y, step_width,
                                 25);
  touch_[kTouchMinus10].Initialize(step_x, step_y + Numpad::kRowHeight,
                                   step_width, 25);
  touch_[kTouchPlus10].Initialize(step_x + step_width + 2,
                                  step_y + Numpad::kRowHeight, step_width, 25);
  // The radial menu: one button in each direction, from Up, clockwise.
  static const s16 kRadial[8][2] = {{114, 30}, {214, 62}, {222, 105},
                                    {214, 148}, {114, 178}, {14, 148},
                                    {6, 105},   {14, 62}};
  for (u32 i = 0; i < 8; i++) {
    touch_[kTouchRadial0 + i].Initialize(kRadial[i][0], kRadial[i][1], 92, 26);
  }
  // A grid of 3 columns and 4 rows for the texts of an entry.
  for (u32 i = 0; i < kMaxChoices; i++) {
    touch_[kTouchChoice0 + i].Initialize(10 + (i % 3) * 101,
                                         kEditorY + (i / 3) * 32, 98, 29);
  }
  // The grid of values, then its bar: <, the views, the filter, >.
  for (u32 i = 0; i < kPickerCells; i++) {
    touch_[kTouchPicker0 + i].Initialize(
        kPickerX + (i % kPickerColumns) * kPickerCellWidth,
        kEditorY + (i / kPickerColumns) * kPickerCellHeight, kPickerCellWidth,
        kPickerCellHeight);
  }
  touch_[kTouchPagePrevious].Initialize(10, kPickerBarY, 40,
                                        kPickerBarHeight);
  touch_[kTouchPickerView].Initialize(54, kPickerBarY, 128, kPickerBarHeight);
  touch_[kTouchPickerFilter].Initialize(186, kPickerBarY, 80,
                                        kPickerBarHeight);
  touch_[kTouchPageNext].Initialize(270, kPickerBarY, 40, kPickerBarHeight);
  // Reset: on the left of Undo.
  touch_[kTouchReset].Initialize(204, 2, 54, 24);
  numpad_.Initialize(10, kEditorY);
  keyboard_.Initialize(10, kEditorY);
}

void MainApplication::PrepareEditor(const PageItem& entry) {
  switch (GetEditor(entry)) {
    case Editor::kNumber: {
      c16 text[32];
      entry.GetNumberText(text);
      numpad_.SetInput(text, entry.IsSigned(), entry.IsDecimal());
      break;
    }
    case Editor::kText:
      keyboard_.SetInput((const c16*)entry.GetAddress(),
                         entry.GetTextCapacity());
      break;
    case Editor::kPicker:
      PreparePicker(entry, GetSelectedIndex());
      break;
    default:
      break;
  }
}

MainApplication::Editor MainApplication::GetEditor(
    const PageItem& entry) const {
  if (entry.IsReadOnly() && entry.GetType() != kTypeMenu) {
    return entry.HasCallback() ? Editor::kRun : Editor::kNone;
  }
  switch (entry.GetType()) {
    case kTypeSeparator:
      return Editor::kNone;
    case kTypeUnicode:
      return Editor::kText;
    case kTypeMenu:
      return Editor::kRun;
    case kTypeIdle:
      return entry.HasCallback() ? Editor::kRun : Editor::kNone;
    case kTypeBoolean:
    case kTypeCheatCode:
      return Editor::kSwitch;
    default:
      break;
  }
  if (UsesPicker(entry)) return Editor::kPicker;
  const u32 size = entry.GetArraySize();
  if (size != 0 && size <= kMaxChoices) return Editor::kChoices;
  return entry.HasValue() ? Editor::kNumber : Editor::kNone;
}

void MainApplication::DrawLabel(s32 x, s32 y, const c16* text,
                                Color color) const {
  DrawMenuText(x, y, text, color);
}

void MainApplication::DrawTouch(TouchId id, const c16* label,
                                bool is_active) const {
  const Button& button = touch_[id];
  const s32 x = button.GetX();
  const s32 y = button.GetY();
  const s32 width = button.GetWidth();
  const s32 height = button.GetHeight();
  const bool is_down = button.IsDown();

  // One rectangle for each button. A pressed button sinks by one pixel.
  Color fill = is_active || is_down ? theme_.selected_text_color
                                    : theme_.unselected_text_color;
  fill.a = is_down ? 0.7f : (is_active ? 0.4f : 0.12f);
  const s32 inset = is_down ? 1 : 0;
  sys::Graphics::DrawRect(x + inset, y + inset, width - 2 * inset,
                          height - 2 * inset, fill);
  // The label is in the center of the button.
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  s32 label_x = x + (width - sys::Graphics::GetTextWidth(label)) / 2;
  if (label_x < x + 2) label_x = x + 2;
  DrawLabel(label_x, y + (height - 12) / 2, label,
            theme_.unselected_text_color);
}

void MainApplication::DrawBottom(sys::Graphics& graphics) {
  painter_->DrawBottomOverlay(graphics);

  if (!IsOpened() || !painter_->ShowBottom()) return;

  // The image of the theme, then the color of the theme over it.
  if (theme_.background_image) {
    GetBottomImage().DrawCentered(
        320, 240, Color(1, 1, 1, theme_.background_image_opacity));
  }
  sys::Graphics::FillScreen(theme_.background_color);
  if (is_radial_open_) {
    DrawRadial();
    return;
  }
  if (entries_count_ == 0) return;

  const u32 index = GetSelectedIndex();
  const PageItem& entry = entries_[index];
  const Editor editor = GetEditor(entry);
  if (is_filtering_) {
    DrawFilterKeyboard(entry);
    return;
  }

  c16 buffer[BUFFER_SIZE];

  // Where the player is: the names of the open pages. The first page shows
  // the name, the version and the author of the plugin.
  c8 path[96] = "Menu";
  for (u32 i = 1; i < contexts_count_; i++) {
    const c8* title = contexts_[i].title;
    if (title == nullptr) continue;
    strncat(path, " > ", sizeof(path) - strlen(path) - 1);
    strncat(path, title, sizeof(path) - strlen(path) - 1);
  }
  // Undo, and Reset when the value changed since the page opened.
  const bool has_reset = IsChanged(index) && entry.CanWriteValue();
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  DrawPart(6, 3, contexts_count_ <= 1 ? kCredit : path,
           has_reset ? 36 : (undo_count_ != 0 ? 46 : kWrapLength),
           theme_.unselected_text_color);
  if (undo_count_ != 0) {
    core::Utils::Format(buffer, u"Undo %u", undo_count_);
    DrawTouch(kTouchUndo, buffer, false);
  }
  if (has_reset) DrawTouch(kTouchReset, u"Reset", false);

  // The selected entry and its description.
  c8 entry_path[128];
  bool is_pinned = false;
  const Shortcut* shortcut = GetShortcut(index);
  if (shortcut != nullptr) {
    is_pinned = FindPin(shortcut->path) >= 0;
  } else if (BuildPath(index, entry_path, sizeof(entry_path))) {
    is_pinned = FindPin(entry_path) >= 0;
  }
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  if (editor == Editor::kPicker && picker_hover_ >= 0) {
    // The value under the stylus: the player sees it before the choice.
    c16 name[48];
    NameList::GetName(entry, picker_hover_, name, SIZE(name));
    core::Utils::Format(buffer, u"> %ls  (%d)", name, picker_hover_);
  } else {
    core::Utils::Format(buffer, u"%s%s", entry.GetName(),
                        is_pinned ? "  [Pinned]" : "");
  }
  DrawLabel(6, 15, buffer, theme_.selected_text_color);
  const c8* description = entry.GetDescription();
  if (description == nullptr) {
    if (entry.IsReadOnly()) {
      description = "This value is for information only.";
    } else if (editor == Editor::kPicker) {
      description = "Touch a value, then lift the stylus. Filter: type a "
                    "part of its name.";
    } else {
      description = GetHint(entry.GetType(), entry.GetArraySize() != 0);
    }
  }
  DrawWrapped(6, 33, description, theme_.unselected_text_color);

  // The editor of the value.
  const s32 value_index = entry.HasValue() ? entry.GetIndex() : -1;
  switch (editor) {
    case Editor::kRun:
      DrawTouch(kTouchRun,
                entry.GetType() == kTypeMenu ? u"Open the page   (A)"
                                             : u"Run   (A)",
                false);
      break;
    case Editor::kSwitch: {
      const bool has_texts = entry.GetArraySize() == 2;
      core::Utils::Format(buffer, u"%s", has_texts ? entry.GetArray()[0] : "Off");
      DrawTouch(kTouchOff, buffer, value_index == 0);
      core::Utils::Format(buffer, u"%s", has_texts ? entry.GetArray()[1] : "On");
      DrawTouch(kTouchOn, buffer, value_index != 0);
      break;
    }
    case Editor::kChoices: {
      const u32 size = entry.GetArraySize();
      for (u32 i = 0; i < size; i++) {
        core::Utils::Format(buffer, u"%s", entry.GetArray()[i]);
        DrawTouch((TouchId)(kTouchChoice0 + i), buffer,
                  (u32)value_index % size == i);
      }
      break;
    }
    case Editor::kNumber: {
      numpad_.Draw();
      DrawTouch(kTouchMinus1, u"-1", false);
      DrawTouch(kTouchPlus1, u"+1", false);
      DrawTouch(kTouchMinus10, u"-10", false);
      DrawTouch(kTouchPlus10, u"+10", false);
      // The limits of the value, at the right of the bar.
      s32 min;
      s32 max;
      const bool has_min = entry.GetMin(min);
      const bool has_max = entry.GetMax(max);
      if (has_min && has_max) {
        core::Utils::Format(buffer, u"%d to %d", min, max);
      } else if (has_min) {
        core::Utils::Format(buffer, u"%d or more", min);
      } else if (has_max) {
        core::Utils::Format(buffer, u"%d or less", max);
      }
      if (has_min || has_max) {
        sys::Graphics::SetTextScale(0.4f, 0.4f);
        sys::Graphics::DrawText(230, kEditorY + 5, buffer,
                                theme_.unselected_text_color);
      }
      break;
    }
    case Editor::kText:
      keyboard_.Draw();
      break;
    case Editor::kPicker:
      DrawPicker(entry, value_index);
      break;
    default:
      break;
  }

  // The state of the game, for the developers. Unmangle() returns the same
  // buffer each time: copy the name of the process before the next call.
  uptr vtable = 0;
  c8 process_name[64];
  strncpy(process_name,
          core::Utils::Unmangle(
              core::ProcessManager::GetInstance().GetCurrentProcessName(
                  vtable)),
          sizeof(process_name) - 1);
  process_name[sizeof(process_name) - 1] = '\0';
  const char* event_name = core::Utils::Unmangle(
      core::EventManager::GetInstance().GetCurrentEventName(vtable));
  core::Utils::Format(buffer, u"Process: %s  |  Event: %s", process_name,
                      event_name);
  sys::Graphics::SetTextScale(0.4f, 0.4f);
  sys::Graphics::DrawText(6, kFooterY, buffer, theme_.unselected_text_color);

  // The navigation bar. Each button also shows its key.
  DrawTouch(kTouchSectionUp, u"L Prev.", false);
  DrawTouch(kTouchSectionDown, u"R Next", false);
  DrawTouch(kTouchPin, is_pinned ? u"Y Unpin" : u"Y Pin", is_pinned);
  DrawTouch(kTouchSearch, u"Search", is_search_page_);
  DrawTouch(kTouchBack, u"B Back", false);
}

void MainApplication::ForceClose() {
  sys::Sound::PlaySoundEffect(IsOpened()
                                ? theme_.close_sound
                                : theme_.open_sound);
  if (IsOpened()) SaveSettings();
  is_opened_ = false;
  is_radial_open_ = 0;
  core::DevicePatch::GetInstance().use_redirection = is_opened_;
}

void MainApplication::ReadTouch(const PageItem& entry, Input& input) {
  if (!painter_->ShowBottom()) return;

  const Editor editor = GetEditor(entry);
  const u8 index = GetSelectedIndex();
  if (editor != editor_ || index != touch_index_) {
    // A touch that started on a different entry does not count.
    for (u32 i = 0; i < kTouchMax; i++) touch_[i].Reset();
    editor_ = editor;
    touch_index_ = index;
  }

  for (u32 i = kTouchSectionUp; i <= kTouchBack; i++) touch_[i].Update();
  if (touch_[kTouchSectionUp].IsReleased()) input.section = -1;
  if (touch_[kTouchSectionDown].IsReleased()) input.section = 1;
  if (touch_[kTouchPin].IsReleased()) input.pin = true;
  if (touch_[kTouchSearch].IsReleased()) input.search = true;
  if (touch_[kTouchBack].IsReleased()) input.back = true;
  if (undo_count_ != 0) {
    touch_[kTouchUndo].Update();
    if (touch_[kTouchUndo].IsReleased()) input.undo = true;
  }
  if (IsChanged(index) && entry.CanWriteValue()) {
    touch_[kTouchReset].Update();
    if (touch_[kTouchReset].IsReleased()) input.reset = true;
  }

  switch (editor) {
    case Editor::kRun:
      touch_[kTouchRun].Update();
      if (touch_[kTouchRun].IsReleased()) input.run = true;
      break;
    case Editor::kSwitch:
      touch_[kTouchOff].Update();
      touch_[kTouchOn].Update();
      if (touch_[kTouchOff].IsReleased()) input.choice = 0;
      if (touch_[kTouchOn].IsReleased()) input.choice = 1;
      break;
    case Editor::kChoices:
      for (u32 i = 0; i < entry.GetArraySize(); i++) {
        touch_[kTouchChoice0 + i].Update();
        if (touch_[kTouchChoice0 + i].IsReleased()) input.choice = i;
      }
      break;
    case Editor::kPicker:
      ReadPickerTouch(entry, input);
      break;
    case Editor::kNumber: {
      static const s32 kSteps[] = {-10, -1, 1, 10};
      for (u32 i = 0; i < 4; i++) {
        touch_[kTouchMinus10 + i].Update();
        if (touch_[kTouchMinus10 + i].IsReleased()) input.step = kSteps[i];
      }
      break;
    }
    default:
      break;
  }
}

void MainApplication::Update(sys::Controller& controller) {
  // The settings file gives the theme and the buttons of the menu: read it
  // before the first frame of the menu.
  if (!are_settings_loaded_) LoadSettings();

  if (AreKeysReleased(controller)) {
    sys::Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
    if (IsOpened()) SaveSettings();
    is_opened_ ^= 1;
    is_radial_open_ = 0;
    core::DevicePatch::GetInstance().use_redirection = is_opened_;
    return;
  }

  if (!IsOpened()) return;
  StepAnimations();
  ItemCategories::Step();

  if (is_radial_open_) {
    UpdateRadial(controller);
    return;
  }
  if (entries_count_ == 0) {
    if (controller.IsKeyReleased(Key::kB)) Close();
    return;
  }
  if (is_filtering_) {
    UpdateFilter(controller);
    return;
  }

  PageItem& entry = GetSelectedEntry();
  // A copy: an action can build the page again.
  const PageItem selected = entry;
  const Editor editor = GetEditor(entry);
  const u8 index = GetSelectedIndex();

  // The numpad and the keyboard start with the value of the entry.
  if (needs_prepare_ || index != prepared_index_) {
    PrepareEditor(entry);
    prepared_index_ = index;
    needs_prepare_ = false;
  }

  if (painter_->ShowBottom()) {
    if (editor == Editor::kText) {
      keyboard_.Update();
      // The search shows its results while the player types.
      if (is_search_page_ && index == 0) {
        const c16* typed = keyboard_.GetInput();
        u32 i = 0;
        while (i + 1 < kQueryLength && typed[i] == search_query_[i] &&
               typed[i] != 0) {
          i++;
        }
        if (typed[i] != search_query_[i]) {
          memset(search_query_, 0, sizeof(search_query_));
          for (i = 0; i + 1 < kQueryLength && typed[i] != 0; i++) {
            search_query_[i] = typed[i];
          }
          Refresh();
          return;
        }
      }
    } else if (editor == Editor::kNumber) {
      numpad_.Update();
    }
  }

  // Read the buttons and the touch screen.
  Input input;
  if (controller.IsKeyDown(Key::kUp) || controller.IsKeyDown(Key::kDown)) {
    hold_frames_++;
  } else {
    hold_frames_ = 0;
  }
  if (controller.IsKeyDown(Key::kLeft) || controller.IsKeyDown(Key::kRight)) {
    hold_step_frames_++;
  } else {
    hold_step_frames_ = 0;
  }
  const s32 speed = hold_frames_ > kFastScrollFrames ? 3 : 1;
  // A held Left / Right changes a number faster. A list of texts stays slow.
  const s32 step = (hold_step_frames_ > kFastStepFrames &&
                    editor == Editor::kNumber)
                     ? 10
                     : 1;
  const bool is_typing = editor == Editor::kNumber || editor == Editor::kText;
  if (controller.IsKeyRepeated(Key::kDown)) input.move = speed;
  if (controller.IsKeyRepeated(Key::kUp)) input.move = -speed;
  if (controller.IsKeyRepeated(Key::kL)) input.section = -1;
  if (controller.IsKeyRepeated(Key::kR)) input.section = 1;
  if (controller.IsKeyRepeated(Key::kRight)) input.step = step;
  if (controller.IsKeyRepeated(Key::kLeft)) input.step = -step;
  if (controller.IsKeyReleased(Key::kB)) input.back = true;
  if (controller.IsKeyReleased(Key::kA)) input.run = true;
  if (controller.IsKeyReleased(Key::kY)) input.pin = true;
  if ((controller.IsKeyReleased(Key::kX) && is_typing) ||
      (editor == Editor::kNumber && numpad_.IsButtonOkReleased()) ||
      (editor == Editor::kText && keyboard_.IsButtonOkReleased())) {
    input.apply = true;
  }
  // X opens the radial menu of the pins, except in the numpad and the
  // keyboard (X applies the value there). The radial menu needs the bottom
  // screen of the painter.
  if (!is_typing && painter_->ShowBottom() &&
      controller.IsKeyPressed(Key::kX)) {
    is_radial_open_ = 1;
    is_radial_held_ = 1;
    radial_direction_ = -1;
    for (u32 i = 0; i < 8; i++) touch_[kTouchRadial0 + i].Reset();
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    return;
  }
  ReadTouch(entry, input);

  // Do the actions.
  if (input.back) {
    Close();
    return;
  }
  if (input.pin) {
    TogglePin(index);
    return;
  }
  if (input.undo) {
    Undo();
    return;
  }
  if (input.search) {
    if (!is_search_page_) {
      sys::Sound::PlaySoundEffect(theme_.confirm_sound);
      Open(LoadSearchPage, nullptr, "Search");
    }
    return;
  }

  // The grid and Reset write the value at once.
  if (input.reset) {
    if (index < snapshot_count_) ApplyValue(index, snapshot_[index], false);
    return;
  }
  if (editor == Editor::kPicker && input.choice >= 0) {
    ApplyValue(index, (u32)input.choice, true);
    return;
  }

  bool is_changed = false;
  const u8 type = entry.GetType();
  const bool is_switch = type == kTypeBoolean || type == kTypeCheatCode;
  const bool can_change = entry.HasValue() && !entry.IsReadOnly();
  if (can_change &&
      (input.step != 0 || input.choice >= 0 || input.apply ||
       (input.run && is_switch && !entry.HasCallback()))) {
    SaveUndo(entry, index);
  }
  if (input.run) {
    if (is_switch && !entry.HasCallback()) {
      if (can_change) {
        sys::Sound::PlaySoundEffect(theme_.confirm_sound);
        entry.Increment();
        is_changed = true;
      } else {
        sys::Sound::PlaySoundEffect(theme_.error_sound);
      }
    } else if (type == kTypeMenu || entry.HasCallback()) {
      sys::Sound::PlaySoundEffect(theme_.confirm_sound);
      if (type != kTypeMenu) AddRecent(index);
      entry.Execute(*this);
      return;
    }
  } else if (input.step != 0 && can_change) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    if (input.step > 0) {
      entry.Increment(input.step);
    } else {
      entry.Decrement(-input.step);
    }
    // The grid shows the page of the new value.
    if (editor == Editor::kPicker) picker_view_ = kViewAll;
    is_changed = true;
  } else if (input.choice >= 0 && can_change) {
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
    const s32 delta = input.choice - entry.GetIndex();
    if (is_switch) {
      if (delta != 0) entry.Increment();
    } else if (delta > 0) {
      entry.Increment(delta);
    } else if (delta < 0) {
      entry.Decrement(-delta);
    }
    is_changed = true;
  } else if (input.apply && can_change) {
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
    if (type == kTypeUnicode) {
      entry.Edit(keyboard_.GetInput());
    } else {
      entry.EditNumber(numpad_.GetInput());
    }
    is_changed = true;
  }

  if (is_changed) {
    flash_index_ = index;
    flash_frames_ = kFlashFrames;
    // First write the data back (OnChange), then build the page again: the
    // page reads the data when it loads.
    AddRecent(index);
    RunOnChange(index);
    if (selected.NeedsRefresh()) Refresh();
    needs_prepare_ = true;
  }

  if (input.move != 0) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    // Wrap only for a new press: a held button stops at the end of the page.
    MoveCursor(input.move, hold_frames_ == 1);
  } else if (input.section != 0) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    JumpSection(input.section);
  }
}

void MainApplication::RunOnChange(u32 index) {
  const callback_t* on_change = &on_change_;
  Shortcut* shortcut = GetShortcut(index);
  if (shortcut != nullptr) on_change = &shortcut->on_change;
  if (*on_change != nullptr) (*on_change)(nullptr);
}

void MainApplication::SaveUndo(const PageItem& entry, u32 index) {
  const u32 size = entry.GetValueSize();
  if (size == 0 || size > sizeof(undo_[0].bytes)) return;
  // A full list forgets the oldest change.
  if (undo_count_ == kMaxUndo) {
    for (u32 i = 0; i + 1 < kMaxUndo; i++) undo_[i] = undo_[i + 1];
    undo_count_--;
  }
  UndoState& state = undo_[undo_count_++];
  state.item = entry;
  state.index = index;
  state.size = size;
  memcpy(state.bytes, entry.GetAddress(), size);
}

void MainApplication::Undo() {
  if (undo_count_ == 0) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  }
  sys::Sound::PlaySoundEffect(theme_.close_sound);
  // The last change first: the changes go back in the opposite order, so
  // each entry finds the data of its time (for example the same slot).
  UndoState& state = undo_[--undo_count_];
  PageItem& item = state.item;
  if (item.GetType() == kTypeCheatCode) {
    // A cheat code runs its code: switch it again.
    item.Increment();
  } else {
    memcpy(item.GetAddress(), state.bytes, state.size);
  }
  flash_index_ = state.index;
  flash_frames_ = kFlashFrames;
  RunOnChange(state.index);
  if (item.NeedsRefresh()) Refresh();
  needs_prepare_ = true;
}

void MainApplication::ApplyValue(u32 index, u64 value, bool is_choice) {
  if (index >= entries_count_) return;
  PageItem& entry = entries_[index];
  if (entry.IsReadOnly() || !entry.CanWriteValue()) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  }
  // A copy: the page can load again.
  const PageItem selected = entry;
  SaveUndo(entry, index);
  entry.WriteValue(value);
  sys::Sound::PlaySoundEffect(theme_.confirm_sound);
  if (is_choice) AddValueRecent(selected, (u32)value);
  flash_index_ = index;
  flash_frames_ = kFlashFrames;
  // First write the data back (OnChange), then build the page again.
  AddRecent(index);
  RunOnChange(index);
  if (selected.NeedsRefresh()) Refresh();
  needs_prepare_ = true;
}

void MainApplication::TakeSnapshot() {
  snapshot_count_ = entries_count_;
  for (u32 i = 0; i < entries_count_; i++) {
    snapshot_[i] = entries_[i].ReadValue();
  }
}

bool MainApplication::IsChanged(u32 index) const {
  if (index >= snapshot_count_ || index >= entries_count_) return false;
  const PageItem& entry = entries_[index];
  // An entry that builds the page again selects the data of the page (a
  // slot, a box): it is not a change of the data.
  if (!entry.HasValue() || entry.IsReadOnly() || entry.NeedsRefresh()) {
    return false;
  }
  return entry.ReadValue() != snapshot_[index];
}

void MainApplication::Select(u32 index) {
  if (index >= entries_count_) return;
  MenuContext& ctx = GetContext();
  u32 offset = ctx.offset;
  if (index < offset) offset = index;
  if (index >= offset + ctx.display_count) {
    offset = index - ctx.display_count + 1;
  }
  // Show the title of the section of the entry.
  if (index > 0 && index - 1 < offset && !entries_[index - 1].IsSelectable()) {
    offset = index - 1;
  }
  ctx.offset = offset;
  ctx.cursor = index - offset;
}

void MainApplication::MoveCursor(s32 delta, bool wrap) {
  const s32 count = entries_count_;
  const s32 direction = delta > 0 ? 1 : -1;
  s32 index = GetSelectedIndex();
  for (s32 n = 0; n < delta * direction; n++) {
    s32 next = index + direction;
    while (next >= 0 && next < count && !entries_[next].IsSelectable()) {
      next += direction;
    }
    if (next < 0 || next >= count) {
      if (!wrap || n > 0) break;
      next = direction > 0 ? 0 : count - 1;
      while (next >= 0 && next < count && !entries_[next].IsSelectable()) {
        next += direction;
      }
      if (next < 0 || next >= count) break;
    }
    index = next;
  }
  Select(index);
}

void MainApplication::JumpSection(s32 direction) {
  const s32 count = entries_count_;
  bool has_sections = false;
  for (s32 i = 0; i < count; i++) {
    if (!entries_[i].IsSelectable()) has_sections = true;
  }
  // Without sections, L and R jump one screen.
  if (!has_sections) {
    MoveCursor(direction * (s32)GetContext().display_count, false);
    return;
  }

  const s32 index = GetSelectedIndex();
  if (direction > 0) {
    s32 i = index + 1;
    while (i < count && entries_[i].IsSelectable()) i++;
    while (i < count && !entries_[i].IsSelectable()) i++;
    if (i >= count) {
      // After the last section, go back to the first entry.
      i = 0;
      while (i < count && !entries_[i].IsSelectable()) i++;
    }
    Select(i);
    return;
  }

  // Go to the start of the section, then to the previous section.
  s32 start = index;
  while (start > 0 && entries_[start - 1].IsSelectable()) start--;
  if (start < index) {
    Select(start);
    return;
  }
  s32 i = start - 1;
  while (i >= 0 && !entries_[i].IsSelectable()) i--;
  if (i < 0) {
    // Before the first section, go to the last section.
    i = count - 1;
    while (i >= 0 && !entries_[i].IsSelectable()) i--;
    if (i < 0) return;
  }
  while (i > 0 && entries_[i - 1].IsSelectable()) i--;
  Select(i);
}

void MainApplication::FinishLoad() {
  needs_prepare_ = true;
  is_filtering_ = false;
  picker_index_ = 0xFF;
  TakeSnapshot();
  MenuContext& ctx = GetContext();
  ctx.display_count =
      entries_count_ > kMaxDisplayCount ? kMaxDisplayCount : entries_count_;
  if (ctx.cursor + ctx.offset >= entries_count_) {
    ctx.cursor = 0;
    ctx.offset = 0;
  }
  if (entries_count_ == 0) return;
  const u32 index = ctx.cursor + ctx.offset;
  if (entries_[index].IsSelectable()) return;
  // The page starts with a title: select the first entry after it.
  for (u32 i = index; i < entries_count_; i++) {
    if (entries_[i].IsSelectable()) {
      Select(i);
      return;
    }
  }
}

void MainApplication::StartTransition(s32 direction) {
  if (!theme_.animations) return;
  transition_ = 1.0f;
  transition_direction_ = direction;
}

void MainApplication::StepAnimations() {
  // The selection bar goes 45 % of the way to the cursor at each frame: the
  // movement takes about 100 ms.
  const f32 target = 5.0f + (f32)(GetContext().cursor * kLineHeight);
  if (!theme_.animations || cursor_y_ < 0) {
    cursor_y_ = target;
  } else {
    cursor_y_ += (target - cursor_y_) * 0.45f;
    const f32 distance = target - cursor_y_;
    if (distance < 0.5f && distance > -0.5f) cursor_y_ = target;
  }

  if (theme_.animations && transition_ > 0.02f) {
    transition_ *= 0.6f;
  } else {
    transition_ = 0;
  }
  if (flash_frames_ > 0) flash_frames_--;
}

void MainApplication::Open(menu_callback_t load_menu, void* args,
                           const c8* title) {
  if (contexts_count_ >= kMaxContexts) return;

  contexts_[contexts_count_].Initialize(load_menu, args, title);
  contexts_count_++;

  entries_count_ = 0;
  no_background_ = 0;
  is_quick_access_ = 0;
  is_search_page_ = 0;
  on_change_ = nullptr;
  process_vtable_ = 0;
  undo_count_ = 0;
  flash_frames_ = 0;
  load_menu(*this, args);
  FinishLoad();
  StartTransition(1);
}

void MainApplication::Close() {
  if (contexts_count_ <= 1) return;

  contexts_count_--;

  MenuContext& ctx = GetContext();

  entries_count_ = 0;
  no_background_ = 0;
  is_quick_access_ = 0;
  is_search_page_ = 0;
  on_change_ = nullptr;
  process_vtable_ = 0;
  undo_count_ = 0;
  flash_frames_ = 0;
  ctx.load_menu(*this, ctx.args);
  FinishLoad();
  StartTransition(-1);
}

void MainApplication::Refresh() {
  MenuContext& ctx = GetContext();
  entries_count_ = 0;
  is_quick_access_ = 0;
  is_search_page_ = 0;
  on_change_ = nullptr;
  ctx.load_menu(*this, ctx.args);
  FinishLoad();
}

bool MainApplication::CheckProcess(uptr vtable) {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(vtable)) {
    if (is_indexing_) {
      // The search and the quick access read the page without the screen:
      // only say that the page is not available.
      index_failed_ = 1;
      return true;
    }
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
    if (app.theme_.background_image) {
      GetTopImage().DrawCentered(
          400, 240, Color(1, 1, 1, app.theme_.background_image_opacity));
    }
    sys::Graphics::FillScreen(app.theme_.background_color);
  }
}

void MainAppPainter::DrawPageItems(MainApplication& app) {
  // The filter shows its results in place of the page.
  if (app.is_filtering_) {
    app.DrawFilterResults();
    return;
  }
  MainApplication::MenuContext& ctx = app.GetContext();
  const Theme& theme = app.theme_;
  const s32 line_height = MainApplication::kLineHeight;

  // A new page slides in from the side and fades in.
  const f32 transition = app.transition_;
  const s32 shift = (s32)(transition * 24.0f) * app.transition_direction_;
  const f32 fade = 1.0f - transition * 0.8f;

  // The selection bar follows the cursor smoothly.
  const s32 bar_y = app.cursor_y_ < 0
                      ? 5 + (s32)ctx.cursor * line_height
                      : (s32)app.cursor_y_;
  // A line of text is 15 pixels high (16 in the space of the texts).
  const s32 row_height = RectY(line_height);
  Color bar = theme.selected_text_color;
  bar.a = 0.18f * fade;
  sys::Graphics::DrawRect(16, RectY(bar_y) + 1, 372, row_height, bar);
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  DrawMenuText(5 + shift, bar_y, u"", theme.selected_text_color);

  c16 name[256];
  c16 value[256];
  static_assert(IconPool::kLineSlotCount >= MainApplication::kMaxDisplayCount,
                "One icon slot for each line of the top screen");
  for (u32 i = 0; i < ctx.display_count; i++) {
    const u32 index = i + ctx.offset;
    const PageItem& entry = app.entries_[index];
    const s32 y = 5 + i * line_height;
    const s32 x = 25 + shift;

    Color accent = theme.selected_text_color;
    accent.a *= fade;
    if (entry.GetType() == kTypeSeparator) {
      const c8* title = entry.GetName();
      if (title != nullptr && title[0] != '\0') {
        // A section: a colored mark, then its name.
        sys::Graphics::DrawRect(RectX(x), RectY(y) + 3, 3, row_height - 3,
                                accent);
        core::Utils::Format(name, u"%s", title);
        sys::Graphics::SetTextScale(0.6f, 0.6f);
        DrawMenuText(x + 8, y, name, accent);
      } else {
        Color line = theme.unselected_text_color;
        line.a = 0.3f * fade;
        sys::Graphics::DrawRect(RectX(x), RectY(y) + row_height / 2, 360, 1,
                                line);
      }
      continue;
    }

    // The flash of an entry that changed.
    if (app.flash_frames_ != 0 && app.flash_index_ == index) {
      Color flash = theme.selected_text_color;
      flash.a = 0.4f * app.flash_frames_ / MainApplication::kFlashFrames;
      sys::Graphics::DrawRect(16, RectY(y) + 1, 372, row_height, flash);
    }

    Color color = ctx.cursor == i ? theme.selected_text_color
                                  : theme.unselected_text_color;
    color.a *= fade;
    if (entry.IsReadOnly()) color.a *= 0.65f;

    // A mark on the left of a value that changed since the page opened.
    if (app.IsChanged(index)) {
      sys::Graphics::DrawRect(RectX(x) - 7, RectY(y) + 5, 4, 5, accent);
    }

    sys::Graphics::SetTextScale(0.6f, 0.6f);
    entry.GetNameText(name);
    entry.GetValueText(value);
    if (value[0] == 0 || entry.GetType() == kTypeIdle) {
      DrawMenuText(x, y, name, color);
      continue;
    }
    // The values are aligned on the right: the eyes find them faster.
    const s32 name_width = sys::Graphics::GetTextWidth(name);
    const s32 value_width = sys::Graphics::GetTextWidth(value);
    const s32 value_x = kValueRight - value_width + shift;
    if (x + name_width + kValueGap > value_x) {
      // No space: the value follows the name on the same line.
      entry.GetDisplayValue(name);
      DrawMenuText(x, y, name, color);
      continue;
    }
    // Two texts: spaces between the name and the value would add the small
    // error of each space, and push the value out of the screen.
    DrawMenuText(x, y, name, color);
    DrawMenuText(value_x, y, value, color);

    // The icon of the value, at half size, on the left of the value.
    const IconKind kind = entry.GetIconKind();
    if (kind == IconKind::kNone) continue;
    const s32 icon_x = RectX(value_x) - kLineIconWidth - 2;
    if (icon_x < RectX(x + name_width)) continue;
    // While Up / Down stay pressed, only the icons that are ready show.
    const u32 icon = entry.GetIconId((u32)entry.GetIndex());
    IconPool& icons = IconPool::GetInstance();
    const s32 slot = icons.Acquire(IconPool::kFirstLineSlot,
                                   IconPool::kLineSlotCount, kind, icon,
                                   app.hold_frames_ < kHoldNoLoadFrames);
    if (slot < 0) continue;
    icons.Draw(slot, kind, icon, icon_x,
               RectY(y) + (row_height - kLineIconHeight) / 2, kLineIconWidth,
               kLineIconHeight, Color(1, 1, 1, fade));
  }

  // The scroll bar: where the visible entries are in the page.
  const u32 count = app.entries_count_;
  if (count > ctx.display_count && ctx.display_count != 0) {
    constexpr s32 kTrackY = 4;
    constexpr s32 kTrackHeight = 222;
    s32 thumb = kTrackHeight * ctx.display_count / count;
    if (thumb < 12) thumb = 12;
    const s32 y = kTrackY + (kTrackHeight - thumb) * ctx.offset /
                                (count - ctx.display_count);
    Color track = theme.unselected_text_color;
    track.a = 0.25f;
    sys::Graphics::DrawRect(394, kTrackY, 2, kTrackHeight, track);
    sys::Graphics::DrawRect(393, y, 4, thumb, theme.selected_text_color);
  }
}
} // namespace ui
