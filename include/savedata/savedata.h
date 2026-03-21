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

#ifndef SANGO_PLUGIN_SAVEDATA_H
#define SANGO_PLUGIN_SAVEDATA_H

#include "system/game_data_manager.h"

namespace savedata {

class Footer;

/**
 * @class SaveData
 * @brief Represents the structure and management of the game's save data.
 * * This class provides access to the raw save buffer, the segment pointers,
 * and the integrity-checking Footer.
 */
class SaveData {
 public:
  /**
   * @brief Accesses the global SaveData instance via the GameDataManager.
   * @return A reference to the active SaveData instance.
   */
  static SaveData& GetInstance() {
    return GameDataManager::GetInstance().GetSavedata();
  }

  /**
   * @brief Retrieves the footer associated with this save data.
   * @return A reference to the Footer object used for integrity validation.
   */
  Footer& GetFooter() { return *footer_; }

  /// @brief Total number of data segments monitored for integrity.
  static constexpr u32 kSegmentCount = 58;

 private:
  void* vtable_;    ///< Pointer to the virtual method table.
  Footer* footer_;  ///< Pointer to the integrity and checksum footer.

  /**
   * @brief Raw save data buffer.
   * Total size: 0x722C8 bytes.
   */
  u8 data_[0x722C8];

  /**
   * @brief Array of pointers to specific data segments within the save.
   */
  void* segments_[kSegmentCount];
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_H