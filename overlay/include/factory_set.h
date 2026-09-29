#pragma once

#include "common.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/ball.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/species.h"
#include "savedata/native/pokemon_param.h"

namespace factory {
struct PokemonSpec {
  pokemon::SpeciesId species;
  pokemon::ItemId item;
  pokemon::AbilityId ability;
  pokemon::Nature nature;
  pokemon::MoveId moves[4];
  u8 evs[6]; // hp, attack, defense, special attack, special defense, speed
  pokemon::Ball ball;
};

pokemon::SpeciesId GetRandomSpecies(const pokemon::SpeciesId* excluded,
                                    u32 excluded_count);
PokemonSpec BuildSpec(pokemon::SpeciesId species,
                      const pokemon::ItemId* used_items, u32 used_count);
void ApplySpec(savedata::PokemonParam* pokemon, const PokemonSpec& spec);
} // namespace factory