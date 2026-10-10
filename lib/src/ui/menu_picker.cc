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
 * @file menu_picker.cc
 * @brief The grid of values and the filter of the menu.
 *
 * The declarations are in ui/main_application.h.
 *
 * The grid shows the values of an entry as icons (or names) on the bottom
 * screen. Its views: Quick (the recent values, then the suggested values),
 * All, and the categories of items. The player chooses a value when the
 * stylus leaves the screen: the name of the value under the stylus shows
 * first, and the player can move the stylus out of the grid to cancel.
 *
 * The filter shows a keyboard on the bottom screen and the values that
 * contain the typed text on the top screen.
 */

#include "ui/main_application.h"

#include <cstring>

#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "system/native/touch_screen.h"
#include "ui/menu_layout.h"
#include "ui/theme.h"

namespace ui {
using namespace layout;

namespace {
// The characters of the name of a value in a cell of the grid.
constexpr u32 kCellNameLength = 9;

// Returns the cell of the grid at a place of the bottom screen, or -1.
s32 GetCellAt(s32 x, s32 y) {
  if (x < kPickerX || y < kEditorY) return -1;
  const s32 column = (x - kPickerX) / kPickerCellWidth;
  const s32 row = (y - kEditorY) / kPickerCellHeight;
  if (column >= (s32)kPickerColumns || row >= (s32)kPickerRows) return -1;
  return row * kPickerColumns + column;
}
} // namespace

const c8* MainApplication::GetViewName(u8 view) {
  if (view == kViewQuick) return "Quick";
  if (view == kViewAll) return "All";
  return ItemCategories::GetName((ItemCategory)(view - kViewCategory));
}

bool MainApplication::UsesPicker(const PageItem& entry) const {
  if (GetPickerCount(entry) == 0) return false;
  if (entry.GetIconKind() != IconKind::kNone) return true;
  const NameSource source = NameList::GetSource(entry);
  if (source == NameSource::kArray) {
    return entry.GetArraySize() > kMaxChoices;
  }
  return source != NameSource::kNone;
}

u32 MainApplication::GetPickerCount(const PageItem& entry) const {
  if (!entry.HasValue()) return 0;
  if (entry.GetArraySize() != 0) return entry.GetArraySize();
  s32 max;
  if (entry.GetMax(max) && max >= 0) return (u32)max + 1;
  return 0;
}

MainApplication::ValueKind MainApplication::GetValueKind(
    const PageItem& entry) const {
  // With an icon for each value, the value is not the icon (a slot).
  if (entry.HasIconIds()) return kValueNone;
  switch (entry.GetIconKind()) {
    case IconKind::kType:
      return kValueType;
    case IconKind::kBall:
      return kValueBall;
    default:
      break;
  }
  switch (entry.GetType()) {
    case kTypeSpecies:
      return kValueSpecies;
    case kTypeItem:
      return kValueItem;
    case kTypeMove:
      return kValueMove;
    case kTypeAbility:
      return kValueAbility;
    default:
      return kValueNone;
  }
}

void MainApplication::AddValueRecent(const PageItem& entry, u32 value) {
  const ValueKind kind = GetValueKind(entry);
  if (kind == kValueNone) return;
  u16* recents = value_recents_[kind];
  u8& count = value_recent_counts_[kind];
  // The value goes first: remove it from its old place.
  u32 position = count;
  for (u32 i = 0; i < count; i++) {
    if (recents[i] == value) {
      position = i;
      break;
    }
  }
  if (position == count && count < kMaxValueRecents) count++;
  if (position >= count) position = count - 1;
  for (u32 i = position; i > 0; i--) recents[i] = recents[i - 1];
  recents[0] = (u16)value;
}

bool MainApplication::IsSuggested(u32 value) const {
  for (u32 i = 0; i < suggestion_count_; i++) {
    if (suggestions_[i] == value) return true;
  }
  return false;
}

void MainApplication::PreparePicker(const PageItem& entry, u32 index) {
  const u32 count = GetPickerCount(entry);
  if (count == 0) return;

  if (index != picker_index_ || entry.GetAddress() != picker_address_) {
    // A new entry: its suggestions, and the first view.
    picker_index_ = index;
    picker_address_ = entry.GetAddress();
    is_picker_views_open_ = false;
    is_picker_touch_ = false;
    picker_was_down_ = false;
    picker_hover_ = -1;
    suggestion_count_ = 0;
    const suggest_t suggest = entry.GetSuggest();
    if (suggest != nullptr) {
      const u32 found = suggest(suggestions_, kMaxSuggestions);
      suggestion_count_ = found < kMaxSuggestions ? found : kMaxSuggestions;
    }
    picker_view_ =
        IsPickerViewAvailable(entry, kViewQuick) ? kViewQuick : kViewAll;
    picker_page_ = 0;
  }
  if (!IsPickerViewAvailable(entry, picker_view_)) picker_view_ = kViewAll;
  // The view of all the values shows the page of the value.
  if (picker_view_ == kViewAll) {
    picker_page_ = ((u32)entry.GetIndex() % count) / kPickerCells;
  }
  BuildPickerView(entry);
}

bool MainApplication::IsPickerViewAvailable(const PageItem& entry,
                                            u8 view) const {
  if (view == kViewAll) return true;
  if (view == kViewQuick) {
    const ValueKind kind = GetValueKind(entry);
    return suggestion_count_ != 0 ||
           (kind != kValueNone && value_recent_counts_[kind] != 0);
  }
  return view < kViewCount && GetValueKind(entry) == kValueItem;
}

void MainApplication::BuildPickerView(const PageItem& entry) {
  picker_list_count_ = 0;
  const u32 count = GetPickerCount(entry);
  auto add = [&](u32 value) {
    if (value >= count || picker_list_count_ >= SIZE(picker_list_)) return;
    for (u32 i = 0; i < picker_list_count_; i++) {
      if (picker_list_[i] == value) return;
    }
    picker_list_[picker_list_count_++] = (u16)value;
  };

  if (picker_view_ == kViewQuick) {
    const ValueKind kind = GetValueKind(entry);
    if (kind != kValueNone) {
      for (u32 i = 0; i < value_recent_counts_[kind]; i++) {
        add(value_recents_[kind][i]);
      }
    }
    for (u32 i = 0; i < suggestion_count_; i++) add(suggestions_[i]);
  } else if (picker_view_ >= kViewCategory && ItemCategories::IsReady()) {
    const ItemCategory category = (ItemCategory)(picker_view_ - kViewCategory);
    for (u32 item = 1; item < count && item < ItemCategories::kItemCount;
         item++) {
      if (ItemCategories::Get(item) == category) {
        picker_list_[picker_list_count_++] = (u16)item;
      }
    }
  }
}

u32 MainApplication::GetViewCount(const PageItem& entry) const {
  if (picker_view_ == kViewAll) return GetPickerCount(entry);
  return picker_list_count_;
}

u32 MainApplication::GetViewValue(u32 position) const {
  if (picker_view_ == kViewAll) return position;
  return position < picker_list_count_ ? picker_list_[position] : 0;
}

s32 MainApplication::GetCellValue(const PageItem& entry, s32 cell) const {
  if (cell < 0) return -1;
  const u32 position = picker_page_ * kPickerCells + cell;
  if (position >= GetViewCount(entry)) return -1;
  return (s32)GetViewValue(position);
}

void MainApplication::ReadPickerTouch(const PageItem& entry, Input& input) {
  // A category of items fills when the index of the items is ready.
  if (picker_view_ >= kViewCategory && picker_list_count_ == 0 &&
      ItemCategories::IsReady()) {
    BuildPickerView(entry);
  }
  const u32 count = GetViewCount(entry);
  const u32 pages = count == 0 ? 1 : (count + kPickerCells - 1) / kPickerCells;
  if (picker_page_ >= pages) picker_page_ = 0;

  // The bar: <, the views, the filter, >.
  touch_[kTouchPagePrevious].Update();
  touch_[kTouchPageNext].Update();
  touch_[kTouchPickerView].Update();
  touch_[kTouchPickerFilter].Update();
  if (touch_[kTouchPagePrevious].IsReleased()) {
    theme_.Play(theme_.next_sound);
    picker_page_ = picker_page_ > 0 ? picker_page_ - 1 : pages - 1;
  }
  if (touch_[kTouchPageNext].IsReleased()) {
    theme_.Play(theme_.next_sound);
    picker_page_ = picker_page_ + 1 < pages ? picker_page_ + 1 : 0;
  }
  if (touch_[kTouchPickerView].IsReleased()) {
    theme_.Play(theme_.next_sound);
    is_picker_views_open_ = !is_picker_views_open_;
    // The pockets of the items: the menu reads them now (the first time).
    if (GetValueKind(entry) == kValueItem) ItemCategories::Request();
    for (u32 i = 0; i < kMaxChoices; i++) touch_[kTouchChoice0 + i].Reset();
  }
  if (touch_[kTouchPickerFilter].IsReleased()) {
    OpenFilter(entry);
    return;
  }

  // The list of the views, in place of the grid.
  if (is_picker_views_open_) {
    u32 button = 0;
    for (u8 view = 0; view < kViewCount && button < kMaxChoices; view++) {
      if (!IsPickerViewAvailable(entry, view)) continue;
      Button& touch = touch_[kTouchChoice0 + button++];
      touch.Update();
      if (!touch.IsReleased()) continue;
      theme_.Play(theme_.confirm_sound);
      picker_view_ = view;
      is_picker_views_open_ = false;
      BuildPickerView(entry);
      picker_page_ = 0;
      if (view == kViewAll) {
        const u32 all = GetPickerCount(entry);
        picker_page_ = ((u32)entry.GetIndex() % all) / kPickerCells;
      }
    }
    return;
  }

  // The cells: the value under the stylus shows, and the stylus chooses it
  // when it leaves the screen. Out of the grid, the stylus cancels.
  sys::TouchScreen& touch_screen = sys::TouchScreen::GetInstance();
  const bool is_down = touch_screen.IsDown();
  if (is_down) {
    const s32 cell = GetCellAt(touch_screen.GetX(), touch_screen.GetY());
    if (!picker_was_down_) is_picker_touch_ = cell >= 0;
    if (is_picker_touch_) {
      const s32 value = GetCellValue(entry, cell);
      if (value != picker_hover_ && value >= 0) {
        theme_.Play(theme_.next_sound);
      }
      picker_hover_ = (s16)value;
    }
  } else if (picker_was_down_) {
    if (is_picker_touch_ && picker_hover_ >= 0) input.choice = picker_hover_;
    is_picker_touch_ = false;
    picker_hover_ = -1;
  }
  picker_was_down_ = is_down;
}

void MainApplication::DrawPickerViews(const PageItem& entry) const {
  u32 button = 0;
  for (u8 view = 0; view < kViewCount && button < kMaxChoices; view++) {
    if (!IsPickerViewAvailable(entry, view)) continue;
    c16 label[24];
    core::Utils::Format(label, u"%s", GetViewName(view));
    DrawTouch((TouchId)(kTouchChoice0 + button++), label, view == picker_view_);
  }
}

void MainApplication::DrawPicker(const PageItem& entry, s32 value) const {
  const u32 count = GetViewCount(entry);
  const u32 pages = count == 0 ? 1 : (count + kPickerCells - 1) / kPickerCells;
  c16 label[48];

  if (is_picker_views_open_) {
    DrawPickerViews(entry);
  } else {
    const IconKind kind = entry.GetIconKind();
    IconPool& icons = IconPool::GetInstance();
    for (u32 i = 0; i < kPickerCells; i++) {
      const u32 position = picker_page_ * kPickerCells + i;
      if (position >= count) break;
      const u32 id = GetViewValue(position);
      const Button& cell = touch_[kTouchPicker0 + i];
      const s32 x = cell.GetX();
      const s32 y = cell.GetY();
      // The current value and the value under the stylus have a background.
      const bool is_hover = picker_hover_ == (s32)id;
      if ((s32)id == value || is_hover) {
        Color fill = theme_.selected_text_color;
        fill.a = is_hover ? 0.7f : 0.4f;
        sys::Graphics::DrawRect(x, y, kPickerCellWidth - 1,
                                kPickerCellHeight - 1, fill);
      }
      // A suggested value has a mark at its top-left corner.
      if (IsSuggested(id)) {
        sys::Graphics::DrawRect(x + 2, y + 2, 5, 5,
                                theme_.selected_text_color);
      }

      IconState state = IconState::kEmpty;
      if (kind != IconKind::kNone) {
        const u32 icon = entry.GetIconId(id);
        // While Left / Right stay pressed, only the icons that are ready.
        const s32 slot =
            icons.Acquire(0, IconPool::kGridSlotCount, kind, icon,
                          hold_step_frames_ < kHoldNoLoadFrames);
        state = slot < 0 ? IconState::kLoading
                         : icons.Draw(slot, kind, icon,
                                      x + (kPickerCellWidth - IconPool::kWidth) / 2,
                                      y + (kPickerCellHeight - IconPool::kHeight) / 2,
                                      IconPool::kWidth, IconPool::kHeight,
                                      Color(1, 1, 1, 1));
      }
      if (state != IconState::kEmpty) continue;
      // No icon: the start of the name of the value.
      c16 name[32];
      NameList::GetName(entry, id, name, kCellNameLength + 1);
      sys::Graphics::SetTextScale(0.35f, 0.35f);
      DrawLabel(x + (kPickerCellWidth - sys::Graphics::GetTextWidth(name)) / 2,
                y + (kPickerCellHeight - 9) / 2, name,
                theme_.unselected_text_color);
    }
    if (count == 0) {
      // An empty view: the index of the items is not ready yet.
      if (picker_view_ >= kViewCategory && !ItemCategories::IsReady()) {
        core::Utils::Format(label, u"Reading the items... %u / %u",
                            ItemCategories::GetProgress(),
                            ItemCategories::kItemCount);
      } else {
        core::Utils::Format(label, u"No values in this view.");
      }
      sys::Graphics::SetTextScale(0.5f, 0.5f);
      DrawLabel(16, kEditorY + 40, label, theme_.unselected_text_color);
    }
  }

  // The bar.
  core::Utils::Format(label, u"%s  %u/%u", GetViewName(picker_view_),
                      picker_page_ + 1, pages);
  DrawTouch(kTouchPagePrevious, u"<", false);
  DrawTouch(kTouchPickerView, label, is_picker_views_open_);
  DrawTouch(kTouchPickerFilter, u"Filter", false);
  DrawTouch(kTouchPageNext, u">", false);
}

void MainApplication::OpenFilter(const PageItem& entry) {
  theme_.Play(theme_.confirm_sound);
  is_filtering_ = true;
  is_picker_views_open_ = false;
  picker_hover_ = -1;
  memset(filter_query_, 0, sizeof(filter_query_));
  keyboard_.SetInput(u"", kQueryLength);
  keyboard_.ShowCharacters(u'a');
  RunFilter(entry);
}

void MainApplication::RunFilter(const PageItem& entry) {
  // The recent values, then the suggestions, come first.
  u16 first[kMaxValueRecents + kMaxSuggestions];
  u32 first_count = 0;
  const ValueKind kind = GetValueKind(entry);
  if (kind != kValueNone) {
    for (u32 i = 0; i < value_recent_counts_[kind]; i++) {
      first[first_count++] = value_recents_[kind][i];
    }
  }
  for (u32 i = 0; i < suggestion_count_; i++) {
    first[first_count++] = suggestions_[i];
  }
  filter_count_ = NameList::Filter(entry, GetPickerCount(entry),
                                   filter_query_, first, first_count,
                                   filter_results_, SIZE(filter_results_));
  filter_cursor_ = 0;
  filter_offset_ = 0;
}

void MainApplication::UpdateFilter(sys::Controller& controller) {
  const PageItem& entry = GetSelectedEntry();
  if (painter_->ShowBottom()) keyboard_.Update();

  // The results change while the player types.
  const c16* typed = keyboard_.GetInput();
  u32 i = 0;
  while (i + 1 < kQueryLength && typed[i] == filter_query_[i] &&
         typed[i] != 0) {
    i++;
  }
  if (typed[i] != filter_query_[i]) {
    memset(filter_query_, 0, sizeof(filter_query_));
    for (i = 0; i + 1 < kQueryLength && typed[i] != 0; i++) {
      filter_query_[i] = typed[i];
    }
    RunFilter(entry);
  }

  if (controller.IsKeyReleased(Key::kB)) {
    theme_.Play(theme_.close_sound);
    is_filtering_ = false;
    return;
  }
  if (controller.IsKeyReleased(Key::kA) ||
      controller.IsKeyReleased(Key::kX) || keyboard_.IsButtonOkReleased()) {
    if (filter_count_ == 0) {
      theme_.Play(theme_.error_sound);
      return;
    }
    is_filtering_ = false;
    ApplyValue(GetSelectedIndex(), filter_results_[filter_cursor_], true);
    return;
  }

  // Up / Down choose a result, L / R jump one screen.
  if (controller.IsKeyDown(Key::kUp) || controller.IsKeyDown(Key::kDown) ||
      controller.IsKeyDown(Key::kL) || controller.IsKeyDown(Key::kR)) {
    filter_hold_frames_++;
  } else {
    filter_hold_frames_ = 0;
  }
  s32 move = 0;
  if (controller.IsKeyRepeated(Key::kDown)) move = 1;
  if (controller.IsKeyRepeated(Key::kUp)) move = -1;
  if (controller.IsKeyRepeated(Key::kR)) move = kFilterLines;
  if (controller.IsKeyRepeated(Key::kL)) move = -(s32)kFilterLines;
  if (move != 0 && filter_count_ != 0) {
    theme_.Play(theme_.next_sound);
    s32 cursor = (s32)filter_cursor_ + move;
    if (cursor < 0) cursor = move == -1 ? filter_count_ - 1 : 0;
    if (cursor >= (s32)filter_count_) {
      cursor = move == 1 ? 0 : filter_count_ - 1;
    }
    filter_cursor_ = (u16)cursor;
  }
  if (filter_cursor_ < filter_offset_) filter_offset_ = filter_cursor_;
  if (filter_cursor_ >= filter_offset_ + kFilterLines) {
    filter_offset_ = filter_cursor_ - kFilterLines + 1;
  }
}

void MainApplication::DrawFilterResults() {
  const PageItem& entry = GetSelectedEntry();
  const s32 line_height = kLineHeight;
  const s32 row_height = RectY(line_height);
  c16 text[64];

  // The first line: the typed text and the number of results.
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  core::Utils::Format(text, u"Filter: %ls_", filter_query_);
  DrawMenuText(25, 5, text, theme_.selected_text_color);
  core::Utils::Format(text, u"%u", filter_count_);
  DrawMenuText(kValueRight - sys::Graphics::GetTextWidth(text), 5, text,
               theme_.unselected_text_color);
  if (filter_count_ == 0) {
    DrawMenuText(25, 5 + line_height, u"No value has this name.",
                 theme_.unselected_text_color);
    return;
  }

  const IconKind kind = entry.GetIconKind();
  IconPool& icons = IconPool::GetInstance();
  for (u32 i = 0; i < kFilterLines; i++) {
    const u32 position = filter_offset_ + i;
    if (position >= filter_count_) break;
    const u32 value = filter_results_[position];
    const s32 y = 5 + (i + 1) * line_height;
    const bool is_selected = position == filter_cursor_;
    if (is_selected) {
      Color bar = theme_.selected_text_color;
      bar.a = 0.18f;
      sys::Graphics::DrawRect(16, RectY(y) + 1, 372, row_height, bar);
    }
    // A suggested value has a mark on its left.
    s32 x = 25;
    if (IsSuggested(value)) {
      sys::Graphics::DrawRect(RectX(x) - 7, RectY(y) + 5, 4, 5,
                              theme_.selected_text_color);
    }
    if (kind != IconKind::kNone) {
      // An icon keeps its slot while it shows: a scroll loads only the new
      // icons. While a button stays pressed, only the icons that are ready.
      const u32 icon = entry.GetIconId(value);
      const s32 slot = icons.Acquire(IconPool::kFirstLineSlot,
                                     IconPool::kLineSlotCount, kind, icon,
                                     filter_hold_frames_ < kHoldNoLoadFrames);
      if (slot >= 0) {
        icons.Draw(slot, kind, icon, RectX(x),
                   RectY(y) + (row_height - kLineIconHeight) / 2,
                   kLineIconWidth, kLineIconHeight, Color(1, 1, 1, 1));
      }
      x += kLineIconWidth * kTextWidth / 400 + 4;
    }
    const Color color = is_selected ? theme_.selected_text_color
                                    : theme_.unselected_text_color;
    sys::Graphics::SetTextScale(0.6f, 0.6f);
    NameList::GetName(entry, value, text, SIZE(text));
    DrawMenuText(x, y, text, color);
    core::Utils::Format(text, u"%u", value);
    DrawMenuText(kValueRight - sys::Graphics::GetTextWidth(text), y, text,
                 color);
  }
}

void MainApplication::DrawFilterKeyboard(const PageItem& entry) {
  c16 text[64];
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  core::Utils::Format(text, u"%s: filter", entry.GetName());
  DrawLabel(6, 15, text, theme_.selected_text_color);
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  DrawLabel(6, 33, u"Type a part of the name (or the number).",
            theme_.unselected_text_color);
  DrawLabel(6, 45, u"Up / Down: choose.  A: OK.  B: back.",
            theme_.unselected_text_color);
  keyboard_.Draw();
}
} // namespace ui
