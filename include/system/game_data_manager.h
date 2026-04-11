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
struct PlayTime;
}  // namespace savedata

namespace overworld {
class ModelManager;
}  // namespace overworld

class GameDataManager {
 public:
  static GameDataManager& GetInstance() {
    return GameManager::GetInstance().GetGameData();
  }

  savedata::SaveData& GetSavedata() const { return *savedata_; }
  savedata::PokemonTeam& GetPokemonTeam() const { return *pokemon_team_; }
  savedata::PlayTime& GetPlayTime() const { return *play_time_; }
  overworld::ModelManager& GetOverworldModelManager() const {
    return *overworld_model_manager_;
  }

  savedata::SaveData* savedata_;
  void* _0[4];
  savedata::PokemonTeam* pokemon_team_;
  savedata::PlayTime* play_time_;
  void* _1;
  overworld::ModelManager* overworld_model_manager_;
};

#endif  // SANGO_PLUGIN_GAME_DATA_MANAGER_H