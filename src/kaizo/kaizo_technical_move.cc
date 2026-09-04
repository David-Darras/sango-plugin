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

#include "common.h"
#include "game/constant/move.h"
#include "game/constant/species.h"
#include "game/global_data/movepool.h"
#include "game/global_data/pokemon.h"
#include "game/global_data/technical_machine.h"

namespace kaizo {
static const MoveID TM_MOVES[100] =
{
    MoveID::kStealthRock, MoveID::kToxicSpikes, MoveID::kStickyWeb,
    MoveID::kRapidSpin, MoveID::kDefog, MoveID::kTrickRoom, MoveID::kTailwind,
    MoveID::kToxic, MoveID::kWillOWisp, MoveID::kThunderWave, MoveID::kTaunt,
    MoveID::kEncore,
    MoveID::kLeechSeed, MoveID::kWhirlwind,
    MoveID::kHaze, MoveID::kRecover, MoveID::kRoost, MoveID::kSynthesis,
    MoveID::kMoonlight, MoveID::kSlackOff, MoveID::kWish,
    MoveID::kRest, MoveID::kSwordsDance, MoveID::kNastyPlot,
    MoveID::kCalmMind, MoveID::kDragonDance, MoveID::kBulkUp, MoveID::kCoil,
    MoveID::kShellSmash, MoveID::kQuiverDance, MoveID::kAgility,
    MoveID::kAutotomize, MoveID::kProtect,
    MoveID::kSubstitute, MoveID::kBatonPass, MoveID::kExtremeSpeed, MoveID::kSuckerPunch,
    MoveID::kBulletPunch, MoveID::kMachPunch, MoveID::kIceShard, MoveID::kAquaJet,
    MoveID::kShadowSneak, MoveID::kFakeOut, MoveID::kVacuumWave, MoveID::kUTurn,
    MoveID::kVoltSwitch, MoveID::kEarthquake,
    MoveID::kCloseCombat, MoveID::kSuperpower, MoveID::kKnockOff, MoveID::kFacade,
    MoveID::kBodySlam, MoveID::kReturn, MoveID::kDrainPunch, MoveID::kCrunch,
    MoveID::kNightSlash, MoveID::kIronHead, MoveID::kMeteorMash, MoveID::kStoneEdge,
    MoveID::kRockSlide, MoveID::kRockBlast, MoveID::kBraveBird, MoveID::kAcrobatics,
    MoveID::kWaterfall, MoveID::kIronTail, MoveID::kPoisonJab, MoveID::kGunkShot,
    MoveID::kCrossPoison, MoveID::kSeedBomb, MoveID::kPowerWhip, MoveID::kWoodHammer,
    MoveID::kBulletSeed, MoveID::kFlareBlitz, MoveID::kOutrage, MoveID::kIceBeam,
    MoveID::kThunderbolt, MoveID::kFlamethrower, MoveID::kFireBlast, MoveID::kOverheat,
    MoveID::kSurf, MoveID::kScald, MoveID::kHydroPump, MoveID::kDracoMeteor,
    MoveID::kDragonPulse, MoveID::kEarthPower, MoveID::kFlashCannon, MoveID::kGigaDrain,
    MoveID::kHurricane, MoveID::kPlayRough, MoveID::kMoonblast, MoveID::kPsychic,
    MoveID::kDarkPulse
};

void PatchTechnicalMoves() {
  auto* table = global_data::TechnicalMachine::GetTable();
  for (u32 i = 0; i < SIZE(TM_MOVES); i++) {
    table[i] = static_cast<u16>(TM_MOVES[i]);
  }
  for (u32 species_id = static_cast<u16>(Species::kBulbasaur);
       species_id <= static_cast<u16>(Species::kVolcanion);
       species_id++) {
    const auto species = static_cast<Species>(species_id);
    const auto form = static_cast<Form>(0);
    auto& movepool = global_data::Movepool::GetInstance(species, form);
    auto& pokemon = global_data::Pokemon::GetInstance(species, form);
    for (u32 tm_index = 0; tm_index < SIZE(TM_MOVES); tm_index++) {
      u32* tm_bits = &pokemon.technical_moves[0];
      u32 array_index = tm_index / 32;
      u32 bit_shift = tm_index % 32;
      if (movepool.contains(static_cast<u16>(TM_MOVES[tm_index]))) {
        tm_bits[array_index] |= (1U << bit_shift);
      } else {
        tm_bits[array_index] &= ~(1U << bit_shift);
      }
    }
  }
}
}