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

#include "undertow/undertow.h"

#include "feature/core/feature_native_script.h"
#include "feature/overworld/feature_map_character.h"
#include "feature/ui/feature_starter_choice.h"
#include "game/constant/model.h"
#include "game/constant/script.h"
#include "game/constant/species.h"
#include "game/global_data/gift_pokemon.h"
#include "game/savedata/overworld_menu.h"
#include "script/context.h"

namespace undertow {
namespace {
using script::Context;
using script::EventFlag;

constexpr u16 kAquaHideout = 93;
constexpr u16 kAquaHideoutEnter = 92;

constexpr Species kAquaRecruitPokemon[feature::StarterChoice::kCount] = {
    Species::kZubat, Species::kPoochyena, Species::kCarvanha
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

  auto& choice = feature::StarterChoice::GetInstance();
  for (u32 i = 0; i < feature::StarterChoice::kCount; i++) {
    choice.candidates[i] = kAquaRecruitPokemon[i];
  }
  const u32 gift_id = s.ChooseStarter();

  if (gift_id < feature::StarterChoice::kCount) {
    auto& gift = global_data::GiftPokemon::GetInstance(gift_id);
    gift.species = static_cast<u32>(kAquaRecruitPokemon[gift_id]);
    gift.form = Form::kNormal;
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
  feature::NativeScript::Register(ScriptId::kAquaBossWelcome, AquaBossWelcome);

  feature::MapCharacter::Empty(kAquaHideout);
  feature::MapCharacter::Empty(kAquaHideoutEnter);

  feature::MapCharacterRequest boss;
  boss.map_id = kAquaHideout;
  boss.model_id = ModelId::kArchie;
  boss.script_id = ScriptId::kAquaBossWelcome;
  boss.tile_x = 29;
  boss.tile_z = 10;
  boss.height = 4;
  boss.facing = overworld::Facing::kDown;
  feature::MapCharacter::Add(boss);

  feature::MapCharacterRequest shelly;
  shelly.map_id = kAquaHideout;
  shelly.model_id = ModelId::kShelly;
  shelly.script_id = ScriptId::kAquaBossWelcome;
  shelly.tile_x = 28;
  shelly.tile_z = 11;
  shelly.height = 4;
  shelly.facing = overworld::Facing::kDown;
  feature::MapCharacter::Add(shelly);

  feature::MapCharacterRequest matt;
  matt.map_id = kAquaHideout;
  matt.model_id = ModelId::kMatt;
  matt.script_id = ScriptId::kAquaBossWelcome;
  matt.tile_x = 30;
  matt.tile_z = 11;
  matt.height = 4;
  matt.facing = overworld::Facing::kDown;
  feature::MapCharacter::Add(matt);

  {
    feature::MapCharacterRequest grunt;
    grunt.map_id = kAquaHideout;
    grunt.model_id = ModelId::kTeamAquaGruntFemale;
    grunt.script_id = ScriptId::kAquaBossWelcome;
    grunt.tile_x = 27;
    grunt.tile_z = 12;
    grunt.height = 3;
    grunt.facing = overworld::Facing::kDown;
    feature::MapCharacter::Add(grunt);
  }

  {
    feature::MapCharacterRequest grunt;
    grunt.map_id = kAquaHideout;
    grunt.model_id = ModelId::kTeamAquaGruntMale;
    grunt.script_id = ScriptId::kAquaBossWelcome;
    grunt.tile_x = 26;
    grunt.tile_z = 13;
    grunt.height = 3;
    grunt.facing = overworld::Facing::kDown;
    feature::MapCharacter::Add(grunt);
  }

  {
    feature::MapCharacterRequest grunt;
    grunt.map_id = kAquaHideout;
    grunt.model_id = ModelId::kTeamAquaGruntFemale;
    grunt.script_id = ScriptId::kAquaBossWelcome;
    grunt.tile_x = 25;
    grunt.tile_z = 12;
    grunt.height = 3;
    grunt.facing = overworld::Facing::kDown;
    feature::MapCharacter::Add(grunt);
  }

  {
    feature::MapCharacterRequest grunt;
    grunt.map_id = kAquaHideout;
    grunt.model_id = ModelId::kTeamAquaGruntMale;
    grunt.script_id = ScriptId::kAquaBossWelcome;
    grunt.tile_x = 31;
    grunt.tile_z = 12;
    grunt.height = 3;
    grunt.facing = overworld::Facing::kDown;
    feature::MapCharacter::Add(grunt);
  }

  {
    feature::MapCharacterRequest grunt;
    grunt.map_id = kAquaHideout;
    grunt.model_id = ModelId::kTeamAquaGruntFemale;
    grunt.script_id = ScriptId::kAquaBossWelcome;
    grunt.tile_x = 32;
    grunt.tile_z = 13;
    grunt.height = 3;
    grunt.facing = overworld::Facing::kDown;
    feature::MapCharacter::Add(grunt);
  }

  {
    feature::MapCharacterRequest grunt;
    grunt.map_id = kAquaHideout;
    grunt.model_id = ModelId::kTeamAquaGruntMale;
    grunt.script_id = ScriptId::kAquaBossWelcome;
    grunt.tile_x = 33;
    grunt.tile_z = 12;
    grunt.height = 3;
    grunt.facing = overworld::Facing::kDown;
    feature::MapCharacter::Add(grunt);
  }
}
}