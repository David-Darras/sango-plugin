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
 * @file type_team.cc
 * @brief A team to test the new types.
 *
 * Six Pokémon of the game get new types, and some moves of the game get the
 * new types too. The first Pokémon has two new types and four moves of four
 * other new types: one screen shows all of them. The others have a new type and
 * a type of the game. A button of the page "Pokemon"
 * (the top page of the menu) gives the party of the player these six Pokémon,
 * at level 100. ORAS only.
 */

#include "custom_battle.h"

#include "core/native/process_manager.h"
#include "pokemon/address.h"
#include "pokemon/constant/form.h"
#include "pokemon/native/move_data.h"
#include "pokemon/native/species_data.h"
#include "savedata/native/pokemon_team.h"
#include "ui/main_application.h"

namespace battle {
namespace {
struct TypedMove {
  MoveId move;
  TypeId type;
};

// The moves of the game that get a new type.
constexpr TypedMove kTypedMoves[] = {
    {MoveId::kPowerGem, kTypeCosmic},     {MoveId::kCosmicPower, kTypeCosmic},
    {MoveId::kMeteorMash, kTypeCosmic},   {MoveId::kMoonlight, kTypeCosmic},
    {MoveId::kHyperVoice, kTypeSound},    {MoveId::kBoomburst, kTypeSound},
    {MoveId::kEchoedVoice, kTypeSound},   {MoveId::kScreech, kTypeSound},
    {MoveId::kFlashCannon, kTypeLight},   {MoveId::kSignalBeam, kTypeLight},
    {MoveId::kDazzlingGleam, kTypeLight}, {MoveId::kAuroraBeam, kTypeLight},
    {MoveId::kDischarge, kTypePlasma},    {MoveId::kChargeBeam, kTypePlasma},
    {MoveId::kZapCannon, kTypePlasma},    {MoveId::kElectroBall, kTypePlasma},
    {MoveId::kDreamEater, kTypeDream},    {MoveId::kHypnosis, kTypeDream},
    {MoveId::kNightmare, kTypeDream},     {MoveId::kYawn, kTypeDream},
    {MoveId::kRoarOfTime, kTypeTime},     {MoveId::kFutureSight, kTypeTime},
    {MoveId::kDoomDesire, kTypeTime},
};

// A Pokémon of the team: its types (a new type and a type of the game) and its
// moves.
struct TeamMember {
  SpeciesId species;
  TypeId type[2];
  MoveId moves[4];
};

constexpr TeamMember kTeam[] = {
    // One Pokémon with two new types and four moves of four other new types:
    // all the new types are on one screen (the choice of the move).
    {SpeciesId::kCelebi, {kTypeCosmic, kTypeTime},
     {kMovePrismLance, kMoveSonicRequiem, kMoveIonStorm, kMoveDreamfall}},
    // A new type and a type of the game.
    {SpeciesId::kExploud, {TypeId::kNormal, kTypeSound},
     {MoveId::kHyperVoice, MoveId::kBoomburst, MoveId::kEchoedVoice,
      MoveId::kScreech}},
    {SpeciesId::kLanturn, {TypeId::kWater, kTypeLight},
     {MoveId::kFlashCannon, MoveId::kSignalBeam, MoveId::kDazzlingGleam,
      MoveId::kAuroraBeam}},
    {SpeciesId::kMagnezone, {kTypePlasma, TypeId::kSteel},
     {MoveId::kZapCannon, MoveId::kDischarge, MoveId::kElectroBall,
      MoveId::kChargeBeam}},
    {SpeciesId::kMusharna, {TypeId::kPsychic, kTypeDream},
     {MoveId::kDreamEater, MoveId::kHypnosis, MoveId::kNightmare,
      MoveId::kYawn}},
    {SpeciesId::kDialga, {TypeId::kSteel, kTypeTime},
     {MoveId::kRoarOfTime, MoveId::kFutureSight, MoveId::kDoomDesire,
      MoveId::kIronHead}},
};

#ifdef GAME_ORAS
// Gives a Pokémon of the team to a slot of the party.
void FillSlot(savedata::PokemonParam* param, const TeamMember& member) {
  param->accessor->Decrypt();
  pokemon::CoreData* core = param->core;
  const pokemon::SpeciesData& data =
      pokemon::SpeciesData::GetInstance(member.species, FormId::kNormal);
  core->species = member.species;
  core->form = FormId::kNormal;
  core->ability = data.ability[0];
  core->item = ItemId::kNone;
  core->use_nickname = false;
  core->SetLevel(100);
  core->SetMaxIVs();
  for (u32 i = 0; i < 4; i++) {
    core->moves[i] = member.moves[i];
    core->pp[i] = pokemon::MoveData::GetInstance(member.moves[i]).base_pp;
    core->pp_up_count[i] = 0;
  }
  param->accessor->Encrypt();
  param->ResetNickname();
  param->UpdateRuntimeData();
}
#endif
} // namespace

void PatchTypedPokemon() {
#ifdef GAME_ORAS
  for (const TypedMove& entry : kTypedMoves) {
    pokemon::MoveData::GetInstance(entry.move).type = entry.type;
  }
  for (const TeamMember& member : kTeam) {
    auto& data =
        pokemon::SpeciesData::GetInstance(member.species, FormId::kNormal);
    data.type[0] = member.type[0];
    data.type[1] = member.type[1];
  }
#endif
}

// The button of the menu: the party gets the six Pokemon.
void GiveTypeTeamFromMenu(void*) {
  if (!core::ProcessManager::IsOverworldActive() ||
      savedata::PokemonTeam::GetInstance().count == 0) {
    ui::MainApplication::GetInstance().ShowToast(
        "Use it in the overworld, with a Pokemon in the team.");
    return;
  }
  GiveTypeTeam();
  ui::MainApplication::GetInstance().ShowToast(
      "The party has the six Pokemon of the new types.");
}

void GiveTypeTeam() {
#ifdef GAME_ORAS
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return;

  // The party has six slots: the game copies the first Pokémon into the new
  // slots, then each slot gets its own Pokémon.
  while (team.count < savedata::PokemonTeam::kMaxSlots) {
    const bool is_added = ((bool (*)(savedata::PokemonTeam*,
                                     savedata::PokemonParam*))
                               pokemon::address::kAddPokemonToTeam)(
        &team, team.pokemons[0]);
    if (!is_added) break;
  }
  for (u32 i = 0; i < team.count && i < SIZE(kTeam); i++) {
    FillSlot(team.pokemons[i], kTeam[i]);
  }
  team.HealAllPokemons();
#endif
}

void AddTypeTestEntries(ui::MainApplication& app) {
  app.AddSection("Type Test")
     .Add("Receive The New Type Team", GiveTypeTeamFromMenu)
     .WithConfirm()
     .WithDescription("Replaces the party with six Pokemon at level 100 that "
                      "have the new types (Cosmic, Sound, Light, Plasma, "
                      "Dream, Time) and moves of these types. The first one "
                      "(Celebi) has two new types and four new moves of four "
                      "other new types. Hold {A}. Use it in the overworld.");
}

} // namespace battle
