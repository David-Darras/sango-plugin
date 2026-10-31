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
 * @file custom_items.cc
 * @brief Example items: Plasma Core and Dream Pillow (held items), Shiny
 * Switch, Ability Switch, Evolution Prism, Devolution Stone and Knife (items that the player
 * uses in the Bag), Philosopher Stone (an immortal holder), and
 * Leftovers (an item of the game that changes).
 *
 * Each item has:
 * 1. a data function: price, Fling...
 * 2. one or more reaction functions and a ReactionTable,
 * 3. one battle::ItemExtension::AddItem() call.
 *
 * See docs/tutorials/15-add-an-item.md.
 */

#include "custom_battle.h"


#include "battle/constant/moment_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/pokemon.h"
#include "battle/native/situation.h"
#include "battle/patch/item_extension.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/native/ability_slot.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/evolution_data.h"
#include "pokemon/native/item_data.h"
#include "pokemon/native/species_data.h"
#include "pokemon/native/utils.h"
#include "pokemon/patch/pre_evolution.h"

namespace battle {
namespace {

// Plasma Core: raises the Sp. Atk of the holder by one stage when it enters the
// battle.
void PlasmaCoreReaction(Listener* self, Controller* controller, UID owner,
                        s32* local_state) {
  if (Situation::Get(SituationKey::kPokemonId) != owner.value) return;

  auto* mut = static_cast<AdjustStatStageMutation*>(
      controller->Create(MutationKind::kAdjustStatStage, owner));
  mut->target_count = 1;
  mut->target_ids[0] = owner;
  mut->stage_kind = StatStageEffectKind::kSpecialAttack;
  mut->stage_delta = 1;
  controller->Apply(mut);
}

// Makes the holder recover a part of its HP. The moment occurs for each
// Pokemon: the function continues only for the holder.
void HealHolder(Controller* controller, UID owner, u16 divisor) {
  if (Situation::Get(SituationKey::kPokemonId) != owner.value) return;

  Pokemon* holder = controller->GetPokemon(owner);
  if (holder == nullptr || holder->hp == 0 || holder->hp >= holder->max_hp) {
    return;
  }

  auto* mut = static_cast<RecoverHpMutation*>(
      controller->Create(MutationKind::kRecoverHp, owner));
  mut->heal_amount = holder->max_hp / divisor > 0 ? holder->max_hp / divisor : 1;
  mut->target_id = owner;
  mut->message.id = MutationMessageId::kNone;
  controller->Apply(mut);
}

// Dream Pillow: the holder recovers 1/16 of its HP at the end of each turn.
void DreamPillowReaction(Listener* self, Controller* controller, UID owner,
                         s32* local_state) {
  HealHolder(controller, owner, 16);
}

// Leftovers (changed): the holder recovers 1/8 of its HP, not 1/16.
void LeftoversReaction(Listener* self, Controller* controller, UID owner,
                       s32* local_state) {
  HealHolder(controller, owner, 8);
}

void PatchPlasmaCoreData(pokemon::ItemData& item) {
  item.price = 3000;
  item.fling_power = 30;
}

void PatchDreamPillowData(pokemon::ItemData& item) {
  item.price = 2000;
  item.fling_power = 20;
}

// Shiny Switch: the player uses it on a Pokemon in the Bag, and the Pokemon
// becomes shiny. A shiny Pokemon becomes normal again.
bool CanUseShinySwitch(const pokemon::CoreData& pokemon) {
  return true;
}

const c16* UseShinySwitch(pokemon::CoreData& pokemon) {
  if (pokemon::Utils::IsShiny(pokemon.id, pokemon.shiny_id)) {
    pokemon.SetShiny(false);
    return u"{pokemon} lost its shine.";
  }
  pokemon.SetShiny(true);
  return u"{pokemon} shines with all the colors!";
}

// Ability Switch: the Pokemon goes to the next ability of its species (first,
// second, hidden, then the first again). A slot that is empty, or that has the
// same ability as the current one, is skipped. The current slot comes from the
// ability of the Pokemon, not from its flags: the flags can be wrong.
u32 FindNextAbilitySlot(const pokemon::CoreData& pokemon, bool* found) {
  const pokemon::SpeciesData& data =
      pokemon::SpeciesData::GetInstance(pokemon.species, pokemon.form);
  u32 current = pokemon::GetAbilitySlot(pokemon);
  for (u32 slot = 0; slot < 3; slot++) {
    if (data.ability[slot] == pokemon.ability) {
      current = slot;
      break;
    }
  }
  for (u32 step = 1; step < 3; step++) {
    const u32 slot = (current + step) % 3;
    if (data.ability[slot] != AbilityId::kNone &&
        data.ability[slot] != pokemon.ability) {
      *found = true;
      return slot;
    }
  }
  *found = false;
  return current;
}

bool CanUseAbilitySwitch(const pokemon::CoreData& pokemon) {
  bool found = false;
  FindNextAbilitySlot(pokemon, &found);
  return found;
}

const c16* UseAbilitySwitch(pokemon::CoreData& pokemon) {
  bool found = false;
  const u32 slot = FindNextAbilitySlot(pokemon, &found);
  if (found) pokemon::SetAbilitySlot(pokemon, slot);
  return u"{pokemon} changed its ability!";
}

// Devolution Stone: the opposite of an evolution stone. A Pokemon that has a
// species before it "evolves" into this species, with the animation of the game.
SpeciesId DevolveIntoPreEvolution(SpeciesId species,
                                  const pokemon::EvolutionData& evolutions) {
  return pokemon::PreEvolution::Get(species);
}

// Evolution Prism: a Pokemon that evolves goes to its first evolution, with any
// method (a level, a trade, a stone...). The game plays the evolution.
SpeciesId EvolveIntoFirstEvolution(SpeciesId species,
                                   const pokemon::EvolutionData& evolutions) {
  const auto& first = evolutions.data[0];
  return first.method != pokemon::EvolutionMethod::kNone ? first.species
                                                         : SpeciesId::kNone;
}

// Knife: the player uses it on a Pokemon in the Bag and the knife cuts it: the
// Pokemon keeps 1 HP. The game has no medicine that removes HP, so the item has
// a use function. The HP stay at 1 at least.
bool CanUseKnife(const pokemon::CoreData& pokemon) {
  return true;
}

const c16* UseKnife(pokemon::CoreData& pokemon) {
  ItemExtension::ChangeHp(-0xFFFF);
  return u"{pokemon} was cut down to 1 HP!";
}

// Philosopher Stone: the holder cannot die. The moves of the other Pokemon have
// no effect on it, a move that would make it faint leaves 1 HP, and the holder
// recovers the HP that it lost (a move, poison, weather...).
// The numbers of two values of the moment. The game counts them one more than
// SituationKey::kNoEffectFlag and SituationKey::kSurvivalReason (the library
// misses a value before them), so the stone uses the numbers of the game.
constexpr SituationKey kNoEffectKey = static_cast<SituationKey>(65);
constexpr SituationKey kSurvivalKey = static_cast<SituationKey>(59);

void PhilosopherStoneBlock(Listener* self, Controller* controller, UID owner,
                           s32* local_state) {
  if (Situation::Get(SituationKey::kMoveRecipientId) != owner.value) return;
  // A move of the holder on itself (Swords Dance...) works.
  if (Situation::Get(SituationKey::kMoveUserId) == owner.value) return;
  Situation::Set(kNoEffectKey, 1);
}

// A move that would make the holder faint leaves it with 1 HP, like Endure.
// The reason 3 is the one of a held item (a Focus Sash).
void PhilosopherStoneEndure(Listener* self, Controller* controller, UID owner,
                            s32* local_state) {
  constexpr s32 kSurvivedByItem = 3;
  // The Pokémon that receives the move: this check has no Pokémon of the moment.
  if (Situation::Get(SituationKey::kMoveRecipientId) != owner.value) return;
  Situation::Set(kSurvivalKey, kSurvivedByItem);
}

void PhilosopherStoneHeal(Listener* self, Controller* controller, UID owner,
                          s32* local_state) {
  Pokemon* holder = controller->GetPokemon(owner);
  if (holder == nullptr || holder->hp == 0 || holder->hp >= holder->max_hp) {
    return;
  }
  auto* mut = static_cast<RecoverHpMutation*>(
      controller->Create(MutationKind::kRecoverHp, owner));
  mut->heal_amount = holder->max_hp - holder->hp;
  mut->target_id = owner;
  mut->message.id = MutationMessageId::kNone;
  controller->Apply(mut);
}

void PatchPhilosopherStoneData(pokemon::ItemData& item) {
  item.price = 0;
  item.fling_power = 0;
}

const ReactionTable kPlasmaCoreReactions[] = {
    {MomentKind::kPokemonEntered, PlasmaCoreReaction},
};
const ReactionTable kDreamPillowReactions[] = {
    {MomentKind::kEndOfTurnChecksBegin, DreamPillowReaction},
};
const ReactionTable kPhilosopherStoneReactions[] = {
    {MomentKind::kImmunityCheckTier2, PhilosopherStoneBlock},
    {MomentKind::kEndureCheck, PhilosopherStoneEndure},
    {MomentKind::kActionEnd, PhilosopherStoneHeal},
    {MomentKind::kEndOfTurnChecksEnd, PhilosopherStoneHeal},
};
const ReactionTable kLeftoversReactions[] = {
    {MomentKind::kEndOfTurnChecksBegin, LeftoversReaction},
};

} // namespace

void RegisterCustomItems() {
  // The icon of a new item is the icon of an item of the game.
  ItemExtension::AddItem(
      {kItemPlasmaCore, u"Plasma Core",
       u"A hot core of plasma. It raises the\nSp. Atk of the holder when it "
       u"enters\na battle.",
       ItemId::kWiseGlasses, PatchPlasmaCoreData, kPlasmaCoreReactions,
       SIZE(kPlasmaCoreReactions)});
  ItemExtension::AddItem(
      {kItemDreamPillow, u"Dream Pillow",
       u"A soft pillow full of dreams. The\nholder recovers a little HP at the "
       u"end\nof each turn.",
       ItemId::kLeftovers, PatchDreamPillowData, kDreamPillowReactions,
       SIZE(kDreamPillowReactions)});

  // Two items that the player uses in the Bag. The icon of its own is the
  // file sdmc:/sango/items/<name>.bclim; ItemId::kHeartScale is the icon when
  // the file is missing.
  ItemExtension::AddItem(
      {kItemShinySwitch, u"Shiny Switch",
       u"A switch that makes a Pokemon shine.\nUse it again to remove the shine.",
       ItemId::kHeartScale, nullptr, nullptr, 0, CanUseShinySwitch,
       UseShinySwitch});
  ItemExtension::AddItem(
      {kItemAbilitySwitch, u"Ability Switch",
       u"A switch that changes the ability.\nUse it on a Pokemon to go to its next "
       u"ability.",
       ItemId::kHeartScale, nullptr, nullptr, 0, CanUseAbilitySwitch,
       UseAbilitySwitch});

  // An evolution stone: it makes any Pokemon that evolves evolve. The item has
  // no use function: the game plays the evolution.
  ItemExtension::AddItem(
      {kItemEvolutionPrism, u"Evolution Prism",
       u"A prism that holds the power of all\nthe evolution stones. Use it on a "
       u"Pokemon\nto make it evolve.",
       ItemId::kHeartScale, nullptr, nullptr, 0, nullptr, nullptr,
       EvolveIntoFirstEvolution});

  // The knife cuts the Pokemon that uses it, with a function of the plugin.
  ItemExtension::AddItem(
      {kItemKnife, u"Knife",
       u"A knife with a curse. A Pokemon that\nuses it is cut and keeps only 1 HP.",
       ItemId::kHeartScale, nullptr, nullptr, 0, CanUseKnife, UseKnife, nullptr,
       true});
  // The opposite of an evolution stone: a Pokemon goes back to its species
  // before, with the animation of the game. The item has no use function.
  ItemExtension::AddItem(
      {kItemDevolutionStone, u"Devolution Stone",
       u"A stone that turns back time. Use it on\na Pokemon to make it go back "
       u"to the\nspecies that it evolves from.",
       ItemId::kHeartScale, nullptr, nullptr, 0, nullptr, nullptr,
       DevolveIntoPreEvolution});
  // A held item that makes the holder immortal.
  ItemExtension::AddItem(
      {kItemPhilosopherStone, u"Philosopher Stone",
       u"A red stone of legend. No move hurts\nthe holder or changes its status, "
       u"and\nit cannot die.",
       ItemId::kHeartScale, PatchPhilosopherStoneData,
       kPhilosopherStoneReactions, SIZE(kPhilosopherStoneReactions)});

  // An item of the game: the name, the data and the icon stay the same, the
  // description and the reactions change.
  ItemExtension::AddItem(
      {ItemId::kLeftovers, nullptr,
       u"A held item that restores 1/8 of the\nmax HP of the holder at the end "
       u"of\neach turn.",
       ItemId::kNone, nullptr, kLeftoversReactions,
       SIZE(kLeftoversReactions)});
}

} // namespace battle
