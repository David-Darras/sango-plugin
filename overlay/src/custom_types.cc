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
 * @file custom_types.cc
 * @brief Example types: Cosmic, Sound, Light, Plasma, Dream and Time.
 *
 * A new type has:
 * 1. a number (19 to 31),
 * 2. a name and an icon (a BCLIM file on the SD card),
 * 3. its effects against the other types, in the type chart.
 *
 * See docs/tutorials/14-add-a-type.md.
 */

#include "custom_battle.h"

#include "battle/constant/type_multiplier.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/type_chart.h"
#include "pokemon/constant/type.h"

namespace battle {
namespace {
// The effect of a type against some types. The other types keep the value x1.
struct Effect {
  TypeId type;
  TypeMultiplier value;
};

constexpr TypeMultiplier k0 = TypeMultiplier::k0;
constexpr TypeMultiplier kHalf = TypeMultiplier::k05;
constexpr TypeMultiplier kDouble = TypeMultiplier::k2;

// For each new type: what it does when it attacks (the types that it hits well
// or badly), then what happens to it when the other types attack it.

constexpr Effect kCosmicAttacks[] = {
    {TypeId::kPsychic, kDouble}, {TypeId::kGhost, kDouble},
    {TypeId::kDragon, kDouble},  {kTypeCosmic, kDouble},
    {TypeId::kSteel, kHalf},     {TypeId::kFairy, kHalf},
};
constexpr Effect kCosmicDefends[] = {
    {TypeId::kDark, kDouble},    {TypeId::kGhost, kDouble},
    {TypeId::kIce, kDouble},     {TypeId::kFairy, kDouble},
    {TypeId::kFire, kHalf},      {TypeId::kPsychic, kHalf},
    {TypeId::kDragon, kHalf},    {TypeId::kFighting, kHalf},
};

constexpr Effect kSoundAttacks[] = {
    {TypeId::kIce, kDouble},     {TypeId::kFlying, kDouble},
    {TypeId::kWater, kDouble},   {TypeId::kFairy, kDouble},
    {TypeId::kSteel, kHalf},     {TypeId::kRock, kHalf},
    {TypeId::kGround, kHalf},    {kTypeSound, kHalf},
};
constexpr Effect kSoundDefends[] = {
    {TypeId::kGround, kDouble},  {TypeId::kRock, kDouble},
    {TypeId::kPsychic, kDouble}, {TypeId::kElectric, kDouble},
    {TypeId::kNormal, kHalf},    {TypeId::kFlying, kHalf},
    {TypeId::kGhost, kHalf},     {TypeId::kSteel, kHalf},
};

constexpr Effect kLightAttacks[] = {
    {TypeId::kDark, kDouble},    {TypeId::kGhost, kDouble},
    {TypeId::kIce, kDouble},     {TypeId::kBug, kDouble},
    {TypeId::kSteel, kHalf},     {TypeId::kDragon, kHalf},
    {kTypeLight, kHalf},
};
constexpr Effect kLightDefends[] = {
    {TypeId::kGhost, kDouble},   {TypeId::kDark, kDouble},
    {TypeId::kRock, kDouble},    {TypeId::kPsychic, kHalf},
    {TypeId::kFairy, kHalf},     {TypeId::kElectric, kHalf},
    {kTypeLight, kHalf},
};

constexpr Effect kPlasmaAttacks[] = {
    {TypeId::kWater, kDouble},   {TypeId::kFlying, kDouble},
    {TypeId::kSteel, kDouble},   {TypeId::kIce, kDouble},
    {TypeId::kGround, k0},       {TypeId::kDragon, kHalf},
    {TypeId::kGrass, kHalf},     {kTypePlasma, kHalf},
};
constexpr Effect kPlasmaDefends[] = {
    {TypeId::kGround, kDouble},  {TypeId::kWater, kDouble},
    {TypeId::kFlying, kHalf},    {TypeId::kSteel, kHalf},
    {TypeId::kElectric, kHalf},  {kTypePlasma, kHalf},
};

constexpr Effect kDreamAttacks[] = {
    {TypeId::kFighting, kDouble}, {TypeId::kPoison, kDouble},
    {TypeId::kNormal, kDouble},   {TypeId::kDark, k0},
    {TypeId::kSteel, kHalf},      {kTypeDream, kHalf},
};
constexpr Effect kDreamDefends[] = {
    {TypeId::kDark, kDouble},     {TypeId::kGhost, kDouble},
    {TypeId::kBug, kDouble},      {TypeId::kFighting, kHalf},
    {TypeId::kPsychic, kHalf},    {kTypeDream, kHalf},
};

constexpr Effect kTimeAttacks[] = {
    {TypeId::kDragon, kDouble},   {TypeId::kSteel, kDouble},
    {TypeId::kPsychic, kDouble},  {TypeId::kFairy, kHalf},
    {TypeId::kNormal, kHalf},     {kTypeTime, kHalf},
};
constexpr Effect kTimeDefends[] = {
    {TypeId::kFairy, kDouble},    {TypeId::kGhost, kDouble},
    {TypeId::kDark, kDouble},     {TypeId::kNormal, kHalf},
    {TypeId::kFighting, kHalf},   {TypeId::kSteel, kHalf},
};

template <u32 AttackCount, u32 DefendCount>
void SetEffects(TypeId type, const Effect (&attacks)[AttackCount],
                const Effect (&defends)[DefendCount]) {
  for (const Effect& effect : attacks) {
    TypeChart::Set(type, effect.type, effect.value);
  }
  for (const Effect& effect : defends) {
    TypeChart::Set(effect.type, type, effect.value);
  }
}
} // namespace

void RegisterCustomTypes() {
  // The icon of a type is the file sdmc:/sango/types/<name>.bclim (for
  // example Cosmic.bclim): a 50 x 20 image (see tools/make_type_icon.py and
  // tools/png_to_bclim.py). The buttons of the moves use the colour of a type of
  // the game, the last field.
  GameExtension::AddType({kTypeCosmic, u"Cosmic",
                          TypeId::kDragon});
  GameExtension::AddType({kTypeSound, u"Sound",
                          TypeId::kWater});
  GameExtension::AddType({kTypeLight, u"Light",
                          TypeId::kElectric});
  GameExtension::AddType({kTypePlasma, u"Plasma",
                          TypeId::kFire});
  GameExtension::AddType({kTypeDream, u"Dream",
                          TypeId::kPsychic});
  GameExtension::AddType({kTypeTime, u"Time",
                          TypeId::kSteel});

  // The chart of the new types. TypeChart::Set() changes one cell. To change
  // all the cells at one time, give a table to GameExtension::SetTypeChart().
  SetEffects(kTypeCosmic, kCosmicAttacks, kCosmicDefends);
  SetEffects(kTypeSound, kSoundAttacks, kSoundDefends);
  SetEffects(kTypeLight, kLightAttacks, kLightDefends);
  SetEffects(kTypePlasma, kPlasmaAttacks, kPlasmaDefends);
  SetEffects(kTypeDream, kDreamAttacks, kDreamDefends);
  SetEffects(kTypeTime, kTimeAttacks, kTimeDefends);
}

} // namespace battle
