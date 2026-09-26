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

#include "config_manager.h"
#include "battle/patch/game_extension.h"
#include "core/patch/game_speed.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/savedata.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
static void Test(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  {
    auto& pkm = team.pokemons[0];
    auto& accessor = *pkm->accessor;
    auto& core = *pkm->core;
    accessor.Decrypt();
    core.species = SpeciesId::kTapuKoko;
    core.ability = kAbilityElectricSurge;
    pokemon::Utils::ConvertToNormal(core.id, &core.shiny_id);
    core.moves[0] = MoveId::kThunderbolt;
    core.moves[1] = MoveId::kVoltSwitch;
    core.moves[2] = MoveId::kDazzlingGleam;
    core.moves[3] = MoveId::kUTurn;
    core.experience = 0x7FFFFFFF;
    core.pp[0] = core.pp[1] = core.pp[2] = core.pp[3] = 99;
    accessor.Encrypt();
  }
  {
    auto& pkm = team.pokemons[1];
    auto& accessor = *pkm->accessor;
    auto& core = *pkm->core;
    accessor.Decrypt();
    core.species = SpeciesId::kTapuLele;
    core.ability = kAbilityPsychicSurge;
    pokemon::Utils::ConvertToNormal(core.id, &core.shiny_id);
    core.moves[0] = MoveId::kPsychic;
    core.moves[1] = MoveId::kMoonblast;
    core.moves[2] = MoveId::kShadowBall;
    core.moves[3] = MoveId::kFocusBlast;
    core.pp[0] = core.pp[1] = core.pp[2] = core.pp[3] = 99;
    core.experience = 0x7FFFFFFF;
    accessor.Encrypt();
  }
  {
    auto& pkm = team.pokemons[2];
    auto& accessor = *pkm->accessor;
    auto& core = *pkm->core;
    accessor.Decrypt();
    core.species = SpeciesId::kTapuBulu;
    core.ability = kAbilityGrassySurge;
    pokemon::Utils::ConvertToNormal(core.id, &core.shiny_id);
    core.moves[0] = MoveId::kWoodHammer;
    core.moves[1] = MoveId::kHornLeech;
    core.moves[2] = MoveId::kSuperpower;
    core.moves[3] = MoveId::kStoneEdge;
    core.pp[0] = core.pp[1] = core.pp[2] = core.pp[3] = 99;
    core.experience = 0x7FFFFFFF;
    accessor.Encrypt();
  }
  {
    auto& pkm = team.pokemons[3];
    auto& accessor = *pkm->accessor;
    auto& core = *pkm->core;
    accessor.Decrypt();
    core.species = SpeciesId::kTapuFini;
    core.ability = kAbilityMistySurge;
    pokemon::Utils::ConvertToNormal(core.id, &core.shiny_id);
    core.moves[0] = MoveId::kScald;
    core.moves[1] = MoveId::kMoonblast;
    core.moves[2] = MoveId::kIceBeam;
    core.moves[3] = MoveId::kCalmMind;
    core.pp[0] = core.pp[1] = core.pp[2] = core.pp[3] = 99;
    core.experience = 0x7FFFFFFF;
    accessor.Encrypt();
  }
}

void LoadTopPage(MainApplication& app, void* args) {
  app.Add("Test", Test)
     .Add("Game Speed", core::GameSpeed::GetInstance().game_speed)
     .WithMin(-10)
     .WithMax(10)
     .Add("Repel", CheatCodeId::kNoEncounter)
     .AddSeparator()
     .Add("Player", LoadPlayerPage)
     .Add("Battle", LoadBattlePage)
     .Add("Overworld", LoadOverworldPage)
     .Add("Pokemon", LoadPokemonPage)
     .Add("Save Data", LoadSaveDataPage)
     .Add("Renderer", LoadRendererPage)
     .Add("Scripts", LoadScriptPage)
     .Add("Title Screen", LoadTitleScreenPage)
     .Add("System", LoadSystemPage)
     .AddSeparator()
     .Add("Plugin Theme", LoadThemePage)
     .Add("Save Config", ConfigManager::Save);
}
} // namespace ui