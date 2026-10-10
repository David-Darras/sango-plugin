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
 * @file icon.h
 * @brief The icons of the game (items, Pokémon, types...), drawn by the
 *        menu.
 */

#pragma once

#include "common.h"

namespace ui {
/// The set of icons that an entry of the menu shows.
enum class IconKind : u8 {
  kNone,
  kItem, ///< The value is an item (ItemId).
  kPokemon, ///< The value is a species (SpeciesId).
  kType, ///< The value is a type (0: Normal to 17: Fairy).
  kBall, ///< The value is a Poké Ball (pokemon::Ball).
  /// The value is a damage category: 0 (status), 1 (physical), 2 (special).
  kMoveCategory,
  /// The value is a status condition: 1 (paralysis) to 5 (poison). The
  /// other values have no icon.
  kStatus,
  kLanguage, ///< The value is a language (1: Japanese to 8: Korean).
};

/// The state of an icon in IconPool.
enum class IconState : u8 {
  kLoading, ///< The icon loads in the next frames.
  kReady, ///< The icon shows.
  kEmpty, ///< The value has no icon.
};

/**
 * @brief Textures that show the icons of the game.
 *
 * The icons come from the archives of the game: the item icons, the Pokémon
 * icons, and the small icons of the layouts (types, damage categories,
 * status conditions, languages), in the language of the game. Each slot is
 * one texture of 64 x 32 pixels, with the icon in its center.
 *
 * The pool makes the texture of a slot the first time that the menu asks
 * for an icon in this slot: the game cannot release a texture. After that,
 * a new icon replaces the pixels of the texture.
 *
 * @code
 * IconPool& icons = IconPool::GetInstance();
 * icons.Request(0, IconKind::kItem, 1); // The Master Ball.
 * icons.Draw(0, IconKind::kItem, 1, x, y, 64, 32, Color(1, 1, 1, 1));
 * @endcode
 */
class IconPool {
public:
  /// The slots of the grid of the bottom screen (MainApplication).
  static constexpr u32 kGridSlotCount = 18;
  /// The slots of the lines of the top screen (MainApplication).
  static constexpr u32 kLineSlotCount = 15;
  /// The first slot of the lines of the top screen.
  static constexpr u32 kFirstLineSlot = kGridSlotCount;
  static constexpr u32 kSlotCount = kGridSlotCount + kLineSlotCount;
  /// The size of one texture, in pixels.
  static constexpr s32 kWidth = 64;
  static constexpr s32 kHeight = 32;

  static IconPool& GetInstance();

  /**
   * @brief Makes the textures and loads the icons that the menu asks for.
   *
   * Call it one time for each frame, while the plugin draws the top screen:
   * a new texture at another time makes the game crash. The function makes
   * one texture in each frame at most.
   * @param can_create_texture false when an other texture was made in this
   *        frame.
   */
  void Prepare(bool can_create_texture);

  /**
   * @brief Asks for an icon in a slot. The icon loads at the next Prepare().
   * @param slot The slot, from 0 to kSlotCount - 1.
   * @param kind The set of icons.
   * @param id The item, the species, the type...
   */
  void Request(u32 slot, IconKind kind, u32 id);

  /**
   * @brief Draws the icon of a slot: the 64 x 32 texture in the rectangle
   *        (x, y, width, height).
   * @return The state of the icon. Only IconState::kReady draws.
   */
  IconState Draw(u32 slot, IconKind kind, u32 id, s32 x, s32 y, s32 width,
                 s32 height, Color color) const;

private:
  struct Slot {
    void* texture = nullptr;
    IconKind kind = IconKind::kNone; ///< The icon in the texture.
    u16 id = 0;
    bool has_image = false; ///< false: the icon does not exist.
    IconKind wanted_kind = IconKind::kNone; ///< The icon of Request().
    u16 wanted_id = 0;

    /// Returns true when the texture shows the icon of Request().
    bool IsDone() const { return kind == wanted_kind && id == wanted_id; }
    /// Remembers that the icon of Request() does not exist.
    void SetEmpty() {
      kind = wanted_kind;
      id = wanted_id;
      has_image = false;
    }
  };

  /// The icons that load in one frame at most (one file each).
  static constexpr u32 kLoadsPerFrame = 3;

  /// Loads the item icon or the Pokémon icon of a slot.
  void LoadFileIcon(Slot& slot);
  /// Loads the icons of a layout archive for all the slots of `kind` (one
  /// file).
  void LoadLayoutIcons(IconKind kind);
  /**
   * @brief Writes a BCLIM image into the texture of a slot, in the center.
   * @param is_palette true for an image with a palette (the item icons and
   *        the Pokémon icons).
   * @return false when the image has a format that the pool cannot read.
   */
  bool WriteImage(Slot& slot, const u8* file, u32 size, bool is_palette);

  Slot slots_[kSlotCount];
  bool is_used_ = false; ///< Request() was called one time.
};
} // namespace ui
