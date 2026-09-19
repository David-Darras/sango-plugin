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
#include "pokemon/constant/item.h"
#include "core/utils.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/savedata.h"
#include "script/constant/script.h"

namespace script {
namespace {
void LittlerootGreeter(Context& s) {
  s.TalkStart();
  s.Talk(u"Hi! You just arrived in Littleroot Town?\n"
      u"This script is written in C++, not in Pawn!");
  s.ShowMessage(u"Do you want a random item for 1000 Pokédollars?");
  const bool wants_item = s.AskYesNo();
  const u32 amount = 1000;
  if (wants_item) {
    if (s.GetMoney() < amount) {
      s.Talk(u"You don't have enough money!");
    } else {
      auto item = core::Utils::GetRandomEnum<ItemId>();
      if (s.CanGiveItem(item)) {
        s.SubMoney(amount);
        s.GiveItem(item);
        s.PlayJingle(kJingleItem);
        s.Talk(u"Here, take good care of it!");
      } else {
        s.Talk(u"Oh, your bag is full...");
      }
    }
  } else {
    s.Talk(u"Suit yourself!");
  }

  s.Face(kTalkTarget, overworld::Facing::kLeft);
  s.Wait(10);
  s.Face(kTalkTarget, overworld::Facing::kRight);
  s.Wait(10);
  s.Face(kTalkTarget,
         (overworld::Facing)s.Call(s.GetNatives().PlayerGetReturnDir));

  s.Talk(u"See ya!");
  s.TalkEnd();
}
} // namespace

#ifdef GAME_XY
void KujiraGreeter(Context& s) {
  s.TalkStart();
  s.Talk(u"This is a new script generated with ZettaD's Kujira plugin.");
  s.Talk(u"I'm going to turn all your Pokémon into shinies.");
  s.TalkEnd();
  s.PlayJingle(kJingleItem);
  auto& team = savedata::PokemonTeam::GetInstance();
  for (u32 i = 0; i < team.count; i++) {
    auto& pkm = team.pokemons[i];
    auto& core = *pkm->core;
    pkm->accessor->Decrypt();
    if (pokemon::Utils::IsShiny(core.id, core.shiny_id)) {
      pokemon::Utils::ConvertToNormal(core.id, &core.shiny_id);
    } else {
      pokemon::Utils::ConvertToShiny(core.id, &core.shiny_id);
    }
    pkm->accessor->Encrypt();
  }
}
#endif

void Install() {
#ifdef GAME_XY
  NativeScript::Register(ScriptId::kKujiraGreeter, KujiraGreeter);

  overworld::MapCharacterRequest greeter;
  greeter.map_id = static_cast<MapId>(264);
  greeter.model_id = ModelId::kTeamFlareAdminMale;
  greeter.script_id = ScriptId::kKujiraGreeter;
  greeter.tile_x = 593;
  greeter.tile_z = 492;
  greeter.facing = overworld::Facing::kRight;
  overworld::MapCharacter::Add(greeter);
#else
  NativeScript::Register(ScriptId::kLittlerootGreeter,
                         LittlerootGreeter);

  overworld::MapCharacterRequest greeter;
  greeter.map_id = MapId::kLittlerootTown;
  greeter.model_id = ModelId::kYoungster;
  greeter.script_id = ScriptId::kLittlerootGreeter;
  greeter.tile_x = 100;
  greeter.tile_z = 163;
  greeter.facing = overworld::Facing::kDown;
  overworld::MapCharacter::Add(greeter);
#endif
}
} // namespace script