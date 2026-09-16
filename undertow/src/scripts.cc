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

#include "script/patch/native_script.h"
#include "overworld/patch/map_character.h"
#include "ui/patch/starter_choice.h"
#include "overworld/constant/model.h"
#include "script/constant/script.h"
#include "pokemon/constant/species.h"
#include "pokemon/native/gift_pokemon_data.h"
#include "savedata/native/overworld_menu.h"

namespace undertow {
namespace {
using script::Context;

constexpr SpeciesId kAquaRecruitPokemon[ui::StarterChoice::kCount] = {
    SpeciesId::kZubat, SpeciesId::kPoochyena, SpeciesId::kCarvanha
};

struct CastMember {
  ModelId model_id;
  u16 tile_x;
  u16 tile_z;
  f32 height;
};

constexpr CastMember kAquaHideoutCast[] = {
    {ModelId::kArchie, 29, 10, 4},
    {ModelId::kShelly, 28, 11, 4},
    {ModelId::kMatt, 30, 11, 4},
    {ModelId::kTeamAquaGruntFemale, 27, 12, 3},
    {ModelId::kTeamAquaGruntMale, 26, 13, 3},
    {ModelId::kTeamAquaGruntFemale, 25, 12, 3},
    {ModelId::kTeamAquaGruntMale, 31, 12, 3},
    {ModelId::kTeamAquaGruntFemale, 32, 13, 3},
    {ModelId::kTeamAquaGruntMale, 33, 12, 3},
};

void AquaBossWelcome(Context& s) {
  s.TalkStart();
  if (s.GetFlag(EventFlag::kStarterGiven)) {
    s.Talk(u"Take good care of your Pokémon, recruit.\n"
        u"Team Aqua is counting on you!");
    s.TalkEnd();
    return;
  }
  s.Talk(u"Welcome aboard, new recruit!\n"
      u"I'm the boss of Team Aqua.");
  s.Talk(u"Every grunt needs a partner to serve the sea.\n"
      u"Choose your Pokémon!");
  s.TalkEnd();

  auto& choice = ui::StarterChoice::GetInstance();
  for (u32 i = 0; i < ui::StarterChoice::kCount; i++) {
    choice.candidates[i] = kAquaRecruitPokemon[i];
  }
  const u32 gift_id = s.ChooseStarter();

  if (gift_id < ui::StarterChoice::kCount) {
    auto& gift = pokemon::GiftPokemonData::GetInstance(gift_id);
    gift.species = kAquaRecruitPokemon[gift_id];
    gift.form = FormId::kNormal;
  }
  s.GivePokemon(gift_id, true);
  s.SetFlag(EventFlag::kStarterGiven);
  s.PlayJingle(script::kJinglePokemon);

  auto& overworld_menu = savedata::OverworldMenu::GetInstance();
  overworld_menu.is_pokemon_list_visible = true;
  overworld_menu.pokemon_list_position = 0;

  s.TalkStart();
  s.Talk(u"A fine choice. Now go and prove yourself!");
  s.TalkEnd();
}
}

void InstallScripts() {
  script::NativeScript::Register(ScriptId::kAquaBossWelcome, AquaBossWelcome);

  overworld::MapCharacter::Empty(MapId::kAquaHideout);
  overworld::MapCharacter::Empty(MapId::kAquaHideoutEntrance);

  for (const auto& member : kAquaHideoutCast) {
    overworld::MapCharacterRequest request;
    request.map_id = MapId::kAquaHideout;
    request.model_id = member.model_id;
    request.script_id = ScriptId::kAquaBossWelcome;
    request.tile_x = member.tile_x;
    request.tile_z = member.tile_z;
    request.height = member.height;
    overworld::MapCharacter::Add(request);
  }
}
}
