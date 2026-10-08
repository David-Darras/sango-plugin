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
 * @file whos_that_pokemon.cc
 * @brief The mini-game "Who's that Pokémon?".
 *
 * The declarations are in overworld/patch/whos_that_pokemon.h.
 */

#include "overworld/patch/whos_that_pokemon.h"

#include <cstring>

#include "overworld/constant/model.h"
#include "overworld/patch/map_character.h"
#include "core/utils.h"
#include "pokemon/native/core_data.h"
#include "pokemon/patch/custom_shop.h"
#include "savedata/native/pokemon_team.h"
#include "script/patch/native_script.h"
#include "ui/log_application.h"
#include "ui/patch/starter_choice.h"

namespace overworld {
namespace {
constexpr u32 kNameCapacity = 16;
constexpr u32 kCenterBall = 1;

c16 Fold(c16 c) {
  if (c >= u'A' && c <= u'Z') return c + 32;
  if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 32;
  return c;
}
} // namespace

void WhosThatPokemon::Initialize() {
  script::NativeScript::Register(ScriptId::kWhosThatPokemon, Run);

  MapCharacterRequest host;
  host.map_id = static_cast<MapId>(6);
  host.model_id = ModelId::kReporter;
  host.script_id = ScriptId::kWhosThatPokemon;
  host.tile_x = 97;
  host.tile_z = 172;
  host.height = 0;
  host.facing = Facing::kDown;
  MapCharacter::Add(host);
}

pokemon::SpeciesId WhosThatPokemon::PickSpecies() {
  const u32 max = GetInstance().max_species;
  return static_cast<pokemon::SpeciesId>(1 + core::Utils::GetRandomValue(max));
}

bool WhosThatPokemon::IsSameName(const c16* typed, const c16* expected) {
  u32 i = 0;
  u32 j = 0;
  while (true) {
    while (typed[i] == u' ') i++;
    while (expected[j] == u' ') j++;
    if (typed[i] == u'\0' || expected[j] == u'\0') break;
    if (Fold(typed[i]) != Fold(expected[j])) return false;
    i++;
    j++;
  }
  return typed[i] == u'\0' && expected[j] == u'\0';
}

void WhosThatPokemon::ShowChoice(script::Context& script,
                                 pokemon::SpeciesId species, bool is_hidden) {
  auto& starter = ui::StarterChoice::GetInstance();
  pokemon::SpeciesId saved[ui::StarterChoice::kCount];
  bool saved_egg[ui::StarterChoice::kCount];
  for (u32 i = 0; i < ui::StarterChoice::kCount; i++) {
    saved[i] = starter.candidates[i];
    saved_egg[i] = starter.is_egg[i];
    starter.candidates[i] = species;
    starter.is_egg[i] = i != kCenterBall;
  }
  GetInstance().is_silhouette_ = is_hidden;
  GetInstance().are_names_hidden_ = is_hidden;
  script.ChooseStarter();
  GetInstance().is_silhouette_ = false;
  GetInstance().are_names_hidden_ = false;
  for (u32 i = 0; i < ui::StarterChoice::kCount; i++) {
    starter.candidates[i] = saved[i];
    starter.is_egg[i] = saved_egg[i];
  }
}

void WhosThatPokemon::Reveal(script::Context& script,
                             pokemon::SpeciesId species, const c16* name,
                             bool has_given_up) {
  c16 message[64];
  u32 length = 0;
  const c16* prefix = has_given_up ? u"It was " : u"Wrong! It was ";
  for (u32 i = 0; prefix[i] != u'\0'; i++) message[length++] = prefix[i];
  for (u32 i = 0; name[i] != u'\0' && length + 2 < SIZE(message); i++) {
    message[length++] = name[i];
  }
  message[length++] = u'!';
  message[length] = u'\0';
  script.Talk(message);
  ShowChoice(script, species, false);
}

bool WhosThatPokemon::AskName(script::Context& script, c16* name,
                              u32 capacity) {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return false;
  auto& slot = *team.pokemons[0];

  pokemon::CoreData backup;
  std::memcpy(&backup, slot.core, sizeof(pokemon::CoreData));

  slot.accessor->Decrypt();
  slot.core->nickname[0] = u'\0';
  slot.core->use_nickname = 1;
  slot.core->species = pokemon::SpeciesId::kUnown;
  slot.core->form = pokemon::FormId::kUnownQuestion;
  slot.accessor->Encrypt();

  GetInstance().are_names_hidden_ = true;
  const bool validated = script.InputPartyNickname(0);
  GetInstance().are_names_hidden_ = false;

  name[0] = u'\0';
  if (validated) {
    slot.accessor->Decrypt();
    u32 length = 0;
    while (length + 1 < capacity && slot.core->nickname[length] != u'\0') {
      name[length] = slot.core->nickname[length];
      length++;
    }
    name[length] = u'\0';
    slot.accessor->Encrypt();
  }
  std::memcpy(slot.core, &backup, sizeof(pokemon::CoreData));
  return validated;
}

void WhosThatPokemon::Run(script::Context& script) {
  script.TalkStart();
  Play(script);
  script.TalkEnd();
}

void WhosThatPokemon::Play(script::Context& script) {
  const pokemon::SpeciesId species = PickSpecies();

  c16 expected_name[kNameCapacity * 2] = {};
  String species_name;
  pokemon::CustomShop::GetSpeciesName(species, &species_name);
  for (u32 i = 0; i + 1 < SIZE(expected_name) &&
                  species_name.GetBuffer()[i] != u'\0'; i++) {
    expected_name[i] = species_name.GetBuffer()[i];
  }

  script.Talk(u"Who's that Pokemon?");
  ShowChoice(script, species, true);

  c16 typed[kNameCapacity] = {};
  if (!AskName(script, typed, kNameCapacity)) {
    Reveal(script, species, expected_name, true);
    return;
  }

  if (!IsSameName(typed, expected_name)) {
    Reveal(script, species, expected_name, false);
    return;
  }

  script.Talk(u"That's right! You win this Pokemon!");
  const pokemon::ShopPokemon reward = {species, pokemon::FormId::kNormal,
                                       static_cast<u8>(GetInstance().reward_level),
                                       0};
  if (pokemon::CustomShop::GivePokemon(reward)) {
    script.PlayJingle(script::kJinglePokemon);
  } else {
    script.Talk(u"But you have no room for it...");
  }
}
} // namespace overworld
