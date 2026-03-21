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

#ifndef SANGO_PLUGIN_MENU_PLUGIN_MENU_H
#define SANGO_PLUGIN_MENU_PLUGIN_MENU_H

#include "keyboard.h"
#include "menu_entry.h"
#include "numpad.h"

namespace menu {

/**
 * @brief Singleton class representing the plugin menu system.
 *
 * Handles the display, input, and management of menu entries,
 * submenus, and context tracking using a stack-based approach.
 */
class PluginMenu {
 public:
  /**
   * @brief Returns the singleton instance of the PluginMenu.
   * @return Reference to the unique PluginMenu instance.
   */
  static PluginMenu &GetInstance() { return instance_; }

  /**
   * @brief Draws the top section of the menu with menu entries.
   */
  void DrawTop();

  /**
   * @brief Draws the bottom section of the menu (numpad or keyboard).
   */
  void DrawBottom();

  /**
   * @brief Updates the menu state and input handling.
   * Must be called once per frame.
   */
  void Update();

  /**
   * @brief Checks if the menu is currently opened.
   * @return True if the menu is visible, false otherwise.
   */
  bool IsOpened() const { return is_opened_; };

  /**
   * @brief Enters a submenu with an optional initialization callback.
   * @param load_menu Function to call when entering the submenu.
   */
  void EnterSubMenu(menu_callback_t load_menu);

  /**
   * @brief Leaves the current submenu and returns to the previous context.
   */
  void LeaveSubMenu();

  PluginMenu &WithArray(const c8 *array[], u32 array_size) {
    entries_[entries_count_ - 1].WithArray(array, array_size);
    return *this;
  }

  PluginMenu &WithCallback(callback_t callback) {
    entries_[entries_count_ - 1].WithCallback(callback);
    return *this;
  }

  /**
   * @brief Adds a simple menu entry by name, address, and type.
   * @param name Display name.
   * @param addr Memory address.
   * @param type Entry type (MenuEntryType).
   * @return Reference to the PluginMenu instance.
   */
  PluginMenu &Add(const c8 *name, void *addr, u8 type) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, type);
    return *this;
  }

  /**
   * @brief Adds a submenu entry.
   * @param name Display name of the submenu.
   * @param menu Callback function to build the submenu.
   * @return Reference to the PluginMenu instance.
   */
  PluginMenu &Add(const c8 *name, menu_callback_t menu) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, (void *)menu, kTypeMenu);
    return *this;
  }

  /**
   * @brief Adds a pointer entry.
   * @param name Display name.
   * @param addr Reference to the pointer.
   * @return Reference to the PluginMenu instance.
   */
  PluginMenu &Add(const c8 *name, void *&addr) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, &addr, kTypePointer);
    return *this;
  }

  /**
   * @brief Adds a boolean entry.
   * @param name Display name.
   * @param addr Reference to the boolean variable.
   * @return Reference to the PluginMenu instance.
   */
  PluginMenu &Add(const c8 *name, bool &addr) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, &addr, kTypeBoolean);
    return *this;
  }

  /**
   * @brief Adds a bitfield entry.
   * @param name Display name.
   * @param addr Address of the integer containing the bits.
   * @param offset Bit offset.
   * @param size Bit size.
   * @return Reference to the PluginMenu instance.
   */
  PluginMenu &Add(const c8 *name, void *addr, u32 offset, u32 size) {
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
   * @return Reference to the PluginMenu instance.
   */
  PluginMenu &Add(const c8 *name, c16 *addr, u32 size) {
    if (entries_count_ < kMaxEntries)
      entries_[entries_count_++].Initialize(name, addr, kTypeUnicode, size);
    return *this;
  }

  /**
   * @brief Macro helper for adding numeric typed entries.
   */
#define ADD(type, type_id)                                      \
  PluginMenu &Add(const c8 *name, type &var) {                  \
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

 private:
  /**
   * @brief Internal structure representing a menu context (for submenus).
   */
  struct MenuContext {
    u8 cursor;                  ///< Current cursor position.
    u8 offset;                  ///< Current scroll offset.
    u8 display_count;           ///< Number of entries to display.
    menu_callback_t load_menu;  ///< Initialization callback.

    /**
     * @brief Default constructor.
     */
    MenuContext()
        : cursor(0), offset(0), display_count(0), load_menu(nullptr) {}

    /**
     * @brief Resets the context with a specific callback.
     * @param menu Function to call for this context.
     */
    void Initialize(menu_callback_t menu) {
      cursor = 0;
      offset = 0;
      display_count = 0;
      this->load_menu = menu;
    }
  };

  /**
   * @brief Private constructor for singleton pattern.
   */
  PluginMenu() : is_opened_(0), entries_count_(0), contexts_count_(0) {}

  /**
   * @brief Returns the currently active context.
   * @return Reference to the top context on the stack.
   */
  MenuContext &GetContext() {
    return contexts_[contexts_count_ > 0 ? contexts_count_ - 1 : 0];
  }

  /**
   * @brief Returns the currently selected menu entry in the current context.
   * @return Reference to the selected MenuEntry.
   */
  MenuEntry &GetSelectedEntry() {
    MenuContext &ctx = GetContext();
    return entries_[ctx.cursor + ctx.offset];
  }

  static constexpr u32 kMaxEntries = 32;
  static constexpr u32 kMaxContexts = 8;
  static constexpr u32 kMaxDisplayCount = 15;
  static constexpr u32 kLineHeight = 16;

  static PluginMenu instance_;  ///< Unique instance.

  u32 is_opened_ : 1;       ///< True if menu is open.
  u32 entries_count_ : 5;   ///< Active entries in current menu.
  u32 contexts_count_ : 3;  ///< Number of submenus in the stack.
  u32 : 23;                 ///< Reserved.

  MenuEntry entries_[kMaxEntries];      ///< Entry pool.
  MenuContext contexts_[kMaxContexts];  ///< Context stack.
  Numpad numpad_;                       ///< Numpad logic.
  Keyboard keyboard_;                   ///< Keyboard logic.
};

}  // namespace menu

#endif  // SANGO_PLUGIN_MENU_PLUGIN_MENU_H