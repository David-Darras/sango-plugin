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

#ifndef SANGO_PLUGIN_GAME_DATA_MANAGER_H
#define SANGO_PLUGIN_GAME_DATA_MANAGER_H
#include "game_manager.h"

namespace savedata {
struct PokemonTeam;
class SaveData;
}  // namespace savedata

/**
 * @class GameDataManager
 * @brief Orchestrates access to the game data systems.
 * * This class serves as an intermediary to access the savedata structures
 * through the main GameManager instance.
 */
class GameDataManager {
 public:
  static GameDataManager& GetInstance() {
    return GameManager::GetInstance().GetGameData();
  }

  savedata::SaveData& GetSavedata() const { return *savedata_; }
  savedata::PokemonTeam& GetPokemonTeam() const { return *pokemon_team_; }

  savedata::SaveData* savedata_;
  void* _0[4];
  savedata::PokemonTeam* pokemon_team_;
};

#endif  // SANGO_PLUGIN_GAME_DATA_MANAGER_H