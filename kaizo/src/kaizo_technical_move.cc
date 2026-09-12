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
static const MoveId TM_MOVES[100] =
{
    MoveId::kStealthRock, MoveId::kToxicSpikes, MoveId::kStickyWeb,
    MoveId::kRapidSpin, MoveId::kDefog, MoveId::kTrickRoom, MoveId::kTailwind,
    MoveId::kToxic, MoveId::kWillOWisp, MoveId::kThunderWave, MoveId::kTaunt,
    MoveId::kEncore,
    MoveId::kLeechSeed, MoveId::kWhirlwind,
    MoveId::kHaze, MoveId::kRecover, MoveId::kRoost, MoveId::kSynthesis,
    MoveId::kMoonlight, MoveId::kSlackOff, MoveId::kWish,
    MoveId::kRest, MoveId::kSwordsDance, MoveId::kNastyPlot,
    MoveId::kCalmMind, MoveId::kDragonDance, MoveId::kBulkUp, MoveId::kCoil,
    MoveId::kShellSmash, MoveId::kQuiverDance, MoveId::kAgility,
    MoveId::kAutotomize, MoveId::kProtect,
    MoveId::kSubstitute, MoveId::kBatonPass, MoveId::kExtremeSpeed, MoveId::kSuckerPunch,
    MoveId::kBulletPunch, MoveId::kMachPunch, MoveId::kIceShard, MoveId::kAquaJet,
    MoveId::kShadowSneak, MoveId::kFakeOut, MoveId::kVacuumWave, MoveId::kUTurn,
    MoveId::kVoltSwitch, MoveId::kEarthquake,
    MoveId::kCloseCombat, MoveId::kSuperpower, MoveId::kKnockOff, MoveId::kFacade,
    MoveId::kBodySlam, MoveId::kReturn, MoveId::kDrainPunch, MoveId::kCrunch,
    MoveId::kNightSlash, MoveId::kIronHead, MoveId::kMeteorMash, MoveId::kStoneEdge,
    MoveId::kRockSlide, MoveId::kRockBlast, MoveId::kBraveBird, MoveId::kAcrobatics,
    MoveId::kWaterfall, MoveId::kIronTail, MoveId::kPoisonJab, MoveId::kGunkShot,
    MoveId::kCrossPoison, MoveId::kSeedBomb, MoveId::kPowerWhip, MoveId::kWoodHammer,
    MoveId::kBulletSeed, MoveId::kFlareBlitz, MoveId::kOutrage, MoveId::kIceBeam,
    MoveId::kThunderbolt, MoveId::kFlamethrower, MoveId::kFireBlast, MoveId::kOverheat,
    MoveId::kSurf, MoveId::kScald, MoveId::kHydroPump, MoveId::kDracoMeteor,
    MoveId::kDragonPulse, MoveId::kEarthPower, MoveId::kFlashCannon, MoveId::kGigaDrain,
    MoveId::kHurricane, MoveId::kPlayRough, MoveId::kMoonblast, MoveId::kPsychic,
    MoveId::kDarkPulse
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