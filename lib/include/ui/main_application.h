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
 * @file main_application.h
 * @brief The menu of the plugin.
 *
 * @see docs/concepts/menu.md
 * @see docs/tutorials/02-add-a-menu-page.md
 */

#pragma once

#include <type_traits>

#include "core/cheat_code_manager.h"
#include "system/native/controller.h"
#include "ui/application.h"
#include "ui/page_item.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/widget/keyboard.h"
#include "ui/widget/numpad.h"

namespace sys {
class Controller;
}

namespace ui {
struct Theme;
/**
 * @brief The menu of the plugin.
 *
 * The menu shows one page at a time. A page is a function that adds entries
 * with Add(). The With functions change the last entry.
 *
 * @code
 * void LoadMyPage(ui::MainApplication& app, void* args) {
 *   app.Add("Speed", speed).WithBounds(1, 4)
 *      .Add("Battle", ui::LoadBattlePage);
 * }
 * @endcode
 */
class MainApplication : public Application {
public:
  friend class Painter;
  friend class MainAppPainter;

  STATIC_INLINE MainApplication& GetInstance() { return instance_; }

  /// Draws the entries of the page on the top screen.
  void DrawTop(sys::Graphics& graphics) override;

  /// Draws the numpad or the keyboard on the bottom screen.
  void DrawBottom(sys::Graphics& graphics) override;

  /// Reads the buttons and updates the menu. Call it one time for each frame.
  void Update(sys::Controller& controller) override;

  /// Returns true when the menu is open.
  bool IsOpened() const { return is_opened_; };

  /// Closes the menu.
  void ForceClose();

  /**
   * @brief Opens a page over the current page (8 pages at most).
   * @param load_menu The page function.
   * @param args A value that the page function receives.
   */
  void Open(menu_callback_t load_menu, void* args = nullptr);

  /// Closes the current page and goes back to the previous page.
  void Close();

  /// Sets the object that draws the menu.
  void SetPainter(Painter& painter) {
    painter_ = &painter;
  }

  /**
   * @brief Shows a text for each value of the last entry (0 = first text).
   * @param array The texts.
   * @param array_size The number of texts.
   */
  MainApplication& WithArray(const c8* array[], u32 array_size) {
    entries_[entries_count_ - 1].WithArray(array, array_size);
    return *this;
  }

  /// Runs a function when the player changes or selects the last entry.
  MainApplication& WithCallback(callback_t callback) {
    entries_[entries_count_ - 1].WithCallback(callback);
    return *this;
  }

  /// Builds the page again when the value of the last entry changes.
  MainApplication& WithRefresh() {
    entries_[entries_count_ - 1].WithRefresh();
    return *this;
  }

  /// Sets the minimum value of the last entry.
  MainApplication& WithMin(s32 min) {
    entries_[entries_count_ - 1].WithMin(min);
    return *this;
  }

  /// Sets the maximum value of the last entry.
  MainApplication& WithMax(s32 max) {
    entries_[entries_count_ - 1].WithMax(max);
    return *this;
  }

  /// Sets the minimum and the maximum value of the last entry.
  MainApplication& WithBounds(u32 min, u32 max) {
    entries_[entries_count_ - 1].WithMin(min);
    entries_[entries_count_ - 1].WithMax(max);
    return *this;
  }

  /// Sets the step of a decimal value.
  MainApplication& WithFactor(f32 factor) {
    entries_[entries_count_ - 1].WithFactor(factor);
    return *this;
  }

  /// Hides the background of the menu on this page.
  MainApplication& WithNoBackground() {
    no_background_ = 1;
    return *this;
  }

  /**
     * @brief Checks that the game is in a process (for example the overworld).
     * @param vtable The vtable of the process, for example overworld::address::kVtable.
     * @return true when the game is in a different process. The page must
     *         then return at once: the menu closes it.
     */
  bool CheckProcess(uptr vtable);

  /// Builds the current page again. The page stays open.
  void Refresh();

  /// Adds an entry that enables or disables a cheat code.
  MainApplication& Add(const c8* name, CheatCodeId id) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_].Initialize(
          name, core::CheatCodeManager::GetInstance().Get(id), kTypeCheatCode);
      entries_count_++;
    }
    return *this;
  }

  /// Adds an entry that runs a function when the player presses A.
  MainApplication& Add(const c8* name, callback_t callback = nullptr) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_].Initialize(name, nullptr, kTypeIdle);
      entries_[entries_count_].WithCallback(callback);
      entries_count_++;
    }
    return *this;
  }

  MainApplication& Add(const c8* name, void* addr, u8 type) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, type);
    return *this;
  }

  /// Adds an entry that opens a page.
  MainApplication& Add(const c8* name, menu_callback_t menu,
                       void* args = nullptr) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_].Initialize(name, (void*)menu, kTypeMenu);
      entries_[entries_count_].WithArgs(args);
      entries_count_++;
    }
    return *this;
  }

  MainApplication& Add(const c8* name, void*& addr) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, &addr, kTypePointer);
    return *this;
  }

  /// Adds an On/Off entry.
  MainApplication& Add(const c8* name, bool& addr) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, &addr, kTypeBoolean);
    return *this;
  }

  /// Adds an entry for some bits of a value.
  MainApplication& Add(const c8* name, void* addr, u32 offset, u32 size) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, kTypeBits, offset,
                                            size);
    return *this;
  }

  /**
* @brief Adds a Unicode string entry.
* @param name Display name.
* @param addr Pointer to the UTF-16 string.
* @param size Maximum length/size of the string.
* @return Reference to the MainApplication instance.
*/
  MainApplication& Add(const c8* name, c16* addr, u32 size) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, kTypeUnicode, size);
    return *this;
  }

  /// Adds an entry that shows the name of a species.
  MainApplication& AddSpecies(const c8* name, u16& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeSpecies);
      WithBounds(0, static_cast<u32>(pokemon::SpeciesId::kCount)-1);
    }
    return *this;
  }

  /// Adds an entry that shows the name of a type.
  MainApplication& AddType(const c8* name, u8& var) {
    static const c8* TYPES[] = {
        "Normal", "Fighting", "Flying", "Poison", "Ground", "Rock",
        "Bug", "Ghost", "Steel", "Fire", "Water", "Grass",
        "Electric", "Psychic", "Ice", "Dragon", "Dark", "Fairy"
    };
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeU8);
      WithArray(TYPES, SIZE(TYPES));
    }
    return *this;
  }

  /// Adds an entry that shows the name of an ability.
  MainApplication& AddAbility(const c8* name, u8& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeAbility);
      WithBounds(0, 0xBF);
    }
    return *this;
  }

  /// Adds an entry that shows the name of a move.
  MainApplication& AddMove(const c8* name, u16& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeMove);
      WithBounds(0, 0x26E);
    }
    return *this;
  }

  /// Adds an entry that shows the name of an item.
  MainApplication& AddItem(const c8* name, u16& var) {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize(name, (void*)&var, kTypeItem);
      WithBounds(0, 775);
    }
    return *this;
  }

  /// Adds an empty line.
  MainApplication& AddSeparator() {
    if (entries_count_ < kMaxEntries) {
      entries_[entries_count_++].Initialize("", nullptr, kTypeSeparator);
    }
    return *this;
  }

#define ADD(type, type_id)                                      \
  MainApplication &Add(const c8 *name, type &var) {                  \
    if (entries_count_ < kMaxEntries)                           \
      entries_[entries_count_++].Initialize(name, (void *)&var, \
                                            kType##type_id);    \
    return *this;                                               \
  }

  ADD(u8, U8)
  ADD(u16, U16)
  ADD(u32, U32)
  ADD(u64, U64)
  ADD(s8, S8)
  ADD(s16, S16)
  ADD(s32, S32)
  ADD(s64, S64)
  ADD(f32, F32)
  ADD(f64, F64)

#undef ADD

  /**
  * @brief Binds an `enum class` field directly, aliasing it to the raw
  * integer widget of its underlying type (same size/representation).
  */
  // The code uses `typename ...::type`: the `_t` aliases do not exist in
  // C++11.
  template <typename Enum,
            typename = typename std::enable_if<std::is_enum<Enum>::value>::type>
  MainApplication& Add(const c8* name, Enum& var) {
    return Add(
        name,
        reinterpret_cast<typename std::underlying_type<Enum>::type&>(var));
  }

  /**
  * @brief Same aliasing trick for the semantic widgets below.
  *
  * Each of them is shared by several distinct `enum class` families that
  * happen to have the same underlying width (e.g. `AddType` serves both
  * `MoveType` and `TypeId`), so a single template per widget replaces
  * what would otherwise be one hand-written overload per enum.
  */
#define ADD_ENUM_ALIAS(method, raw)                                       \
  template <typename Enum, typename =                                     \
            typename std::enable_if<std::is_enum<Enum>::value>::type>     \
  MainApplication& method(const c8* name, Enum& var) {                    \
    static_assert(sizeof(Enum) == sizeof(raw),                            \
                  #method " requires an enum of the same width as " #raw);\
    return method(name, reinterpret_cast<raw&>(var));                     \
  }

  ADD_ENUM_ALIAS(AddSpecies, u16)
  ADD_ENUM_ALIAS(AddType, u8)
  ADD_ENUM_ALIAS(AddAbility, u8)
  ADD_ENUM_ALIAS(AddMove, u16)
  ADD_ENUM_ALIAS(AddItem, u16)

#undef ADD_ENUM_ALIAS

private:
  struct MenuContext {
    u8 cursor;
    u8 offset;
    u8 display_count;
    menu_callback_t load_menu;
    void* args;

    MenuContext()
      : cursor(0),
        offset(0),
        display_count(0),
        load_menu(nullptr),
        args(nullptr) {
    }

    void Initialize(menu_callback_t menu, void* args) {
      cursor = 0;
      offset = 0;
      display_count = 0;
      this->load_menu = menu;
      this->args = args;
    }
  };

  MainApplication()
    : is_opened_(0),
      entries_count_(0),
      contexts_count_(0),
      no_background_(0),
      process_vtable_(0),
      theme_(Theme::GetInstance()) {
  }

  MenuContext& GetContext() {
    return contexts_[contexts_count_ > 0 ? contexts_count_ - 1 : 0];
  }

  PageItem& GetSelectedEntry() {
    MenuContext& ctx = GetContext();
    return entries_[ctx.cursor + ctx.offset];
  }

  bool AreKeysReleased(sys::Controller& ctrl);

  static constexpr u32 kMaxEntries = 64;
  static constexpr u32 kMaxContexts = 8;
  static constexpr u32 kMaxDisplayCount = 15;
  static constexpr u32 kLineHeight = 16;

  static MainApplication instance_;

  // The counters can reach kMaxEntries (64) and kMaxContexts (8):
  // 7 bits and 4 bits.
  u32 is_opened_ : 1;
  u32 entries_count_ : 7;
  u32 contexts_count_ : 4;
  u32 no_background_ : 1;
  u32  : 19;

  uptr process_vtable_;

  PageItem entries_[kMaxEntries];
  MenuContext contexts_[kMaxContexts];
  Numpad numpad_;
  Keyboard keyboard_;

  Theme& theme_;
  Painter* painter_ = nullptr;
};
} // namespace ui
